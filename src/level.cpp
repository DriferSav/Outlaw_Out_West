#include <level.hpp>
#include <globals.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
using json = nlohmann::json;

static constexpr Color DEFAULT_COLOR = { 139, 90, 43, 255 };

Level level;

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
        spawn.x = d["spawn"].value("x", 160.0f);
        spawn.y = d["spawn"].value("y", 480.0f);
    }
    if (d.contains("bounds")) {
        worldSize.x = d["bounds"].value("width",  1280.0f);
        worldSize.y = d["bounds"].value("height",  720.0f);
    }

    if (d.contains("objects") && d["objects"].is_array()) {
        for (const auto& obj : d["objects"]) {
            try { ParseObjectInto(obj, objects, solidColliders); }
            catch (const json::exception& e) { std::cerr << "[Level] Skipping bad object: " << e.what() << "\n"; }
        }
    }

    if (d.contains("items") && d["items"].is_array()) {
        for (const auto& it : d["items"]) {
            LevelItem item;
            item.id       = it.value("id", "");
            item.bounds   = { it.at("x").get<float>(), it.at("y").get<float>(), 24.0f, 24.0f };
            item.color    = it.contains("color") ? ParseColor(it["color"]) : Color{220,170,30,255};
            item.collected = false;
            items.push_back(item);
        }
    }

    if (d.contains("enemies") && d["enemies"].is_array()) {
        for (const auto& e : d["enemies"]) {
            EnemySpawn es;
            es.type            = e.value("type", "patrol");
            es.x               = e.value("x", 0.0f);
            es.y               = e.value("y", 0.0f);
            es.hp              = e.value("hp", 2);
            es.damage          = e.value("damage", 1);
            es.speed           = e.value("speed", 80.0f);
            es.patrolLeft      = e.value("patrol_left",  es.x - 128.0f);
            es.patrolRight     = e.value("patrol_right", es.x + 128.0f);
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
    std::cout << "[Level] Loaded '" << name << "' — " << objects.size() << " objects\n";
    return true;
}

static void ParseObjectInto(
    const json& obj,
    std::vector<LevelObject>& objects,
    std::vector<Rectangle>&   solidColliders)
{
    std::string typeStr = obj.value("type", "solid");
    LevelObjectType type = LevelObjectType::SOLID;
    if      (typeStr == "one_sided") type = LevelObjectType::ONE_SIDED;
    else if (typeStr == "gate")      type = LevelObjectType::GATE;
    else if (typeStr == "hazard")    type = LevelObjectType::HAZARD;

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
    lo.isExit  = obj.value("is_exit", false);

    objects.push_back(lo);
}

void Level::BuildColliderLists() {
    solidColliders.clear();
    oneSidedColliders.clear();
    gateColliders.clear();

    for (const auto& o : objects) {
        if (o.type == LevelObjectType::SOLID)
            solidColliders.push_back(o.bounds);
        else if (o.type == LevelObjectType::ONE_SIDED)
            oneSidedColliders.push_back(o.bounds);
        else if (o.type == LevelObjectType::GATE) {
            // Exit gates stay solid always — player must interact to win
            // Regular gates: only solid if not yet unlocked
            if (o.isExit) {
                gateColliders.push_back(o.bounds);  // always solid
            } else if (Global::gameData.unlockedGates.find(o.id) == Global::gameData.unlockedGates.end()) {
                gateColliders.push_back(o.bounds);  // solid until unlocked
            }
        }
        // HAZARD is never a collider
    }
    for (const auto& g : gateColliders)
        solidColliders.push_back(g);
}

int Level::GetContactDamage(Rectangle bounds) const {
    for (const auto& o : objects) {
        if (o.type != LevelObjectType::HAZARD) continue;
        if (CheckCollisionRecs(bounds, o.bounds)) return o.damage;
    }
    return 0;
}

bool Level::TryInteractGate(Rectangle playerBounds,
                              const std::vector<std::string>& inventory) const {
    for (const auto& o : objects) {
        if (o.type != LevelObjectType::GATE || !o.isExit) continue;
        // Expand the gate rect slightly so the player doesn't have to be pixel-perfect
        Rectangle expanded = { o.bounds.x - 16, o.bounds.y,
                               o.bounds.width + 32, o.bounds.height };
        if (!CheckCollisionRecs(playerBounds, expanded)) continue;
        // Check inventory for the required key
        auto it = std::find(inventory.begin(), inventory.end(), o.keyId);
        if (it != inventory.end()) return true;  // WIN
    }
    return false;
}

bool Level::TryUnlockGate(const std::string& keyId) {
    bool anyUnlocked = false;
    for (auto& o : objects) {
        if (o.type != LevelObjectType::GATE) continue;
        if (o.isExit) continue;              // exit gates never auto-unlock
        if (o.keyId != keyId) continue;
        if (Global::gameData.unlockedGates.count(o.id)) continue;
        Global::gameData.unlockedGates.insert(o.id);
        anyUnlocked = true;
    }
    if (anyUnlocked) BuildColliderLists();
    return anyUnlocked;
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
        // Skip unlocked non-exit gates
        if (o.type == LevelObjectType::GATE && !o.isExit &&
            Global::gameData.unlockedGates.count(o.id)) continue;

        Color c = o.color;
        if (o.type == LevelObjectType::HAZARD) c = { 220, 60, 20, 255 };

        DrawRectangleRec(o.bounds, c);
        DrawRectangleLinesEx(o.bounds, 1.0f, { 0,0,0,50 });

        if (o.type == LevelObjectType::HAZARD)
            DrawText("^", (int)(o.bounds.x + o.bounds.width/2 - 4),
                     (int)(o.bounds.y), 14, WHITE);

        if (o.type == LevelObjectType::GATE) {
            // Draw a key hint only if player doesn't have the key yet
            DrawText("G", (int)(o.bounds.x + o.bounds.width/2 - 5),
                     (int)(o.bounds.y + o.bounds.height/2 - 7), 14, GOLD);
            if (o.isExit)
                DrawText("[F]", (int)(o.bounds.x - 4),
                         (int)(o.bounds.y - 18), 12, YELLOW);
        }
    }
    for (const auto& item : items) {
        if (item.collected) continue;
        DrawRectangleRec(item.bounds, item.color);
        DrawText("K", (int)(item.bounds.x + 7), (int)(item.bounds.y + 5), 12, WHITE);
    }
}

void Level::Unload() {
    objects.clear(); items.clear(); enemySpawns.clear();
    solidColliders.clear(); oneSidedColliders.clear(); gateColliders.clear();
    name = "unnamed"; spawn = { 160.0f, 480.0f };
    worldSize = { 99999.0f, 99999.0f }; // safe fallback — death check never triggers until level loads
    loaded = false;
}
