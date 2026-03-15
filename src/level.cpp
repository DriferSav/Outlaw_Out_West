#include <level.hpp>
#include <globals.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
using json = nlohmann::json;

static constexpr Color DEFAULT_COLOR = { 139, 90, 43, 255 };

Level level;

// Forward declarations — defined later in this file
static Color ParseColor(const json& j, Color def = DEFAULT_COLOR);
static void  ParseObjectInto(const json& obj,
                              std::vector<LevelObject>& objects,
                              std::vector<Rectangle>&   solidColliders);

static Color ParseColor(const json& j, Color def) {
    if (j.is_array() && j.size() == 4)
        return { (unsigned char)j[0].get<int>(), (unsigned char)j[1].get<int>(),
                 (unsigned char)j[2].get<int>(), (unsigned char)j[3].get<int>() };
    return def;
}

bool Level::Load(const std::string& path) {
    Unload();
    std::ifstream f(path);
    if (!f.is_open()) { std::cerr << "[Level] Can't open: " << path << "\n"; return false; }
    json d;
    try { d = json::parse(f); }
    catch (const json::exception& e) { std::cerr << "[Level] Parse error: " << e.what() << "\n"; return false; }

    name = d.value("name", "unnamed");
    if (d.contains("spawn")) {
        spawn.x = d["spawn"].value("x", 100.0f);
        spawn.y = d["spawn"].value("y", 200.0f);
    }

    // --- Objects ---
    if (d.contains("objects") && d["objects"].is_array()) {
        for (const auto& obj : d["objects"]) {
            try { ParseObjectInto(obj, objects, solidColliders); }
            catch (const json::exception& e) { std::cerr << "[Level] Skipping bad object: " << e.what() << "\n"; }
        }
    }

    // --- Items ---
    if (d.contains("items") && d["items"].is_array()) {
        for (const auto& it : d["items"]) {
            LevelItem item;
            item.id       = it.value("id", "");
            item.bounds   = { it.at("x").get<float>(), it.at("y").get<float>(), 16.0f, 16.0f };
            item.color    = it.contains("color") ? ParseColor(it["color"]) : Color{220,170,30,255};
            item.collected = false;
            items.push_back(item);
        }
    }

    // --- Enemy spawns ---
    if (d.contains("enemies") && d["enemies"].is_array()) {
        for (const auto& e : d["enemies"]) {
            EnemySpawn es;
            es.type            = e.value("type", "patrol");
            es.x               = e.value("x", 0.0f);
            es.y               = e.value("y", 0.0f);
            es.hp              = e.value("hp", 2);
            es.damage          = e.value("damage", 1);
            es.speed           = e.value("speed", 80.0f);
            es.patrolLeft      = e.value("patrol_left",  es.x - 80.0f);
            es.patrolRight     = e.value("patrol_right", es.x + 80.0f);
            es.jumpInterval    = e.value("jump_interval", 2.0f);
            es.jumpForce       = e.value("jump_force", 380.0f);
            es.throwInterval   = e.value("throw_interval", 3.0f);
            es.projectileSpeed = e.value("projectile_speed", 180.0f);
            es.arcHeight       = e.value("arc_height", 80.0f);
            if (e.contains("path") && e["path"].is_array()) {
                int n = 0;
                for (const auto& wp : e["path"]) {
                    if (n >= EnemySpawn::MAX_PATH) break;
                    es.path[n++] = { wp.value("x",0.0f), wp.value("y",0.0f) };
                }
                es.pathCount = n;
            }
            enemySpawns.push_back(es);
        }
    }

    BuildColliderLists();
    loaded = true;
    std::cout << "[Level] Loaded '" << name << "' with " << objects.size() << " objects\n";
    return true;
}

// Static free function — keeps C++17 compat (no 'auto' parameter)
// Takes the level object list by reference so it can push into it
static void ParseObjectInto(
    const json& obj,
    std::vector<LevelObject>& objects,
    std::vector<Rectangle>& solidColliders)
{
    std::string typeStr = obj.value("type", "solid");
    LevelObjectType type = LevelObjectType::SOLID;
    if      (typeStr == "one_sided") type = LevelObjectType::ONE_SIDED;
    else if (typeStr == "hostile")   type = LevelObjectType::HOSTILE;
    else if (typeStr == "gate")      type = LevelObjectType::GATE;
    else if (typeStr == "hazard")    type = LevelObjectType::HAZARD;
    else if (typeStr == "stair")     type = LevelObjectType::STAIR;

    LevelObject lo;
    lo.type    = type;
    lo.bounds  = { obj.at("x").get<float>(), obj.at("y").get<float>(),
                   obj.at("width").get<float>(), obj.at("height").get<float>() };
    lo.color   = obj.contains("color") ? ParseColor(obj["color"]) : DEFAULT_COLOR;
    lo.texture = obj.value("texture", "");
    lo.id      = obj.value("id", "");
    lo.keyId   = obj.value("key_id", "");
    lo.damage  = obj.value("damage", 1);
    lo.facing  = obj.value("facing", "up");

    objects.push_back(lo);

    // Stairs generate child step solids immediately
    if (type == LevelObjectType::STAIR) {
        const int   steps   = 4;
        const bool  goRight = lo.facing != "left";
        const float sw      = lo.bounds.width  / steps;
        const float sh      = lo.bounds.height / steps;
        for (int i = 0; i < steps; i++) {
            LevelObject step;
            step.type  = LevelObjectType::SOLID;
            step.color = lo.color;
            float xOff = goRight ? i * sw : (steps - 1 - i) * sw;
            step.bounds = {
                lo.bounds.x + xOff,
                lo.bounds.y + lo.bounds.height - (i + 1) * sh,
                sw, (i + 1) * sh
            };
            objects.push_back(step);
            solidColliders.push_back(step.bounds);
        }
    }
}

void Level::BuildColliderLists() {
    solidColliders.clear();
    oneSidedColliders.clear();
    gateColliders.clear();

    for (const auto& o : objects) {
        if (o.type == LevelObjectType::SOLID || o.type == LevelObjectType::HOSTILE)
            solidColliders.push_back(o.bounds);
        else if (o.type == LevelObjectType::ONE_SIDED)
            oneSidedColliders.push_back(o.bounds);
        else if (o.type == LevelObjectType::GATE) {
            // Gate is solid unless already unlocked in this session
            if (Global::gameData.unlockedGates.find(o.id) == Global::gameData.unlockedGates.end())
                gateColliders.push_back(o.bounds);
        }
        // HAZARD + STAIR (steps already added) don't add more here
    }
    // Add locked gates to solid colliders
    for (const auto& g : gateColliders)
        solidColliders.push_back(g);
}

int Level::GetContactDamage(Rectangle bounds) const {
    for (const auto& o : objects) {
        if (o.type != LevelObjectType::HOSTILE && o.type != LevelObjectType::HAZARD) continue;
        if (CheckCollisionRecs(bounds, o.bounds)) return o.damage;
    }
    return 0;
}

bool Level::TryUnlockGate(const std::string& keyId) {
    for (auto& o : objects) {
        if (o.type != LevelObjectType::GATE) continue;
        if (o.keyId != keyId) continue;
        if (Global::gameData.unlockedGates.count(o.id)) continue;
        Global::gameData.unlockedGates.insert(o.id);
        BuildColliderLists(); // rebuild so gate is no longer solid
        return true;
    }
    return false;
}

std::vector<std::string> Level::CollectItems(Rectangle bounds) {
    std::vector<std::string> collected;
    for (auto& item : items) {
        if (item.collected) continue;
        if (CheckCollisionRecs(bounds, item.bounds)) {
            item.collected = true;
            collected.push_back(item.id);
        }
    }
    return collected;
}

void Level::Draw() const {
    for (const auto& o : objects) {
        // Skip STAIR parent rect — child steps draw as SOLID
        if (o.type == LevelObjectType::STAIR) continue;
        // Skip unlocked gates
        if (o.type == LevelObjectType::GATE &&
            Global::gameData.unlockedGates.count(o.id)) continue;

        Color c = o.color;
        if (o.type == LevelObjectType::HOSTILE) c = { 220, 60, 20, 255 }; // always red-orange
        if (o.type == LevelObjectType::HAZARD)  c = { 220, 60, 20, 255 };

        DrawRectangleRec(o.bounds, c);
        DrawRectangleLinesEx(o.bounds, 1.0f, { 0,0,0,50 });

        // Visual label for hazards (until textures are wired up)
        if (o.type == LevelObjectType::HAZARD)
            DrawText("^", (int)(o.bounds.x + o.bounds.width/2 - 4), (int)(o.bounds.y), 12, WHITE);
        if (o.type == LevelObjectType::GATE)
            DrawText("G", (int)(o.bounds.x + o.bounds.width/2 - 4), (int)(o.bounds.y + o.bounds.height/2 - 6), 12, GOLD);
    }
    // Draw uncollected items
    for (const auto& item : items) {
        if (item.collected) continue;
        DrawRectangleRec(item.bounds, item.color);
        DrawText("K", (int)(item.bounds.x + 4), (int)(item.bounds.y + 2), 10, WHITE);
    }
}

void Level::Unload() {
    objects.clear(); items.clear(); enemySpawns.clear();
    solidColliders.clear(); oneSidedColliders.clear(); gateColliders.clear();
    name = "unnamed"; spawn = {100.0f, 200.0f}; loaded = false;
}
