#include <level.hpp>
#include <nlohmann/json.hpp>    // nlohmann/json — single header, must be in include/
#include <fstream>
#include <iostream>
 
using json = nlohmann::json;
 
// ---------------------------------------------------------------------------
// Default fallback color if "color" key is absent in a JSON object
// ---------------------------------------------------------------------------
static constexpr Color DEFAULT_PLATFORM_COLOR = { 139, 90, 43, 255 }; // brown
 
Level level;
 
// ---------------------------------------------------------------------------
// Level::Load
// ---------------------------------------------------------------------------
bool Level::Load(const std::string& path) {
    Unload(); // clear any previously loaded data
 
    // --- Open file ---
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Level] Failed to open: " << path << "\n";
        return false;
    }
 
    // --- Parse JSON ---
    json data;
    try {
        data = json::parse(file);
    } catch (const json::parse_error& e) {
        std::cerr << "[Level] JSON parse error in " << path << ": " << e.what() << "\n";
        return false;
    }
 
    // --- name (optional) ---
    if (data.contains("name")) {
        name = data.at("name").get<std::string>();
    }
 
    // --- spawn (optional) ---
    if (data.contains("spawn")) {
        const auto& sp = data.at("spawn");
        spawn.x = sp.value("x", 100.0f);
        spawn.y = sp.value("y", 200.0f);
    }
 
    // --- objects (required) ---
    if (!data.contains("objects") || !data.at("objects").is_array()) {
        std::cerr << "[Level] Missing or invalid 'objects' array in: " << path << "\n";
        return false;
    }
 
    for (const auto& obj : data.at("objects")) {
        // Required fields — will throw json::out_of_range if missing
        try {
            LevelObject lo;
            lo.bounds.x      = obj.at("x").get<float>();
            lo.bounds.y      = obj.at("y").get<float>();
            lo.bounds.width  = obj.at("width").get<float>();
            lo.bounds.height = obj.at("height").get<float>();
 
            // Optional "color": [R, G, B, A]
            if (obj.contains("color") && obj.at("color").is_array()
                && obj.at("color").size() == 4)
            {
                const auto& c = obj.at("color");
                lo.color = {
                    static_cast<unsigned char>(c[0].get<int>()),
                    static_cast<unsigned char>(c[1].get<int>()),
                    static_cast<unsigned char>(c[2].get<int>()),
                    static_cast<unsigned char>(c[3].get<int>()),
                };
            } else {
                lo.color = DEFAULT_PLATFORM_COLOR;
            }
 
            objects.push_back(lo);
            colliders.push_back(lo.bounds); // keep parallel flat list
        }
        catch (const json::exception& e) {
            std::cerr << "[Level] Skipping malformed object: " << e.what() << "\n";
            // continue loading the rest of the level
        }
    }
 
    loaded = true;
    std::cout << "[Level] Loaded '" << name << "' — "
              << objects.size() << " objects from " << path << "\n";
    return true;
}
 
// ---------------------------------------------------------------------------
// Level::Unload
// ---------------------------------------------------------------------------
void Level::Unload() {
    objects.clear();
    colliders.clear();
    name   = "unnamed";
    spawn  = { 100.0f, 200.0f };
    loaded = false;
}
 
// ---------------------------------------------------------------------------
// Level::Draw
// ---------------------------------------------------------------------------
void Level::Draw() const {
    for (const LevelObject& obj : objects) {
        DrawRectangleRec(obj.bounds, obj.color);
        // Optional: draw a 1px darker outline so platforms are legible
        DrawRectangleLinesEx(obj.bounds, 1.0f, { 0, 0, 0, 60 });
    }
}
 
