#pragma once
#include <raylib.h>
#include <string>
#include <vector>
 
// ---------------------------------------------------------------------------
// LevelObject — one drawable, solid rectangle in the level
// ---------------------------------------------------------------------------
struct LevelObject {
    Rectangle bounds;   // position and size (Raylib Rectangle = x,y,w,h)
    Color     color;    // draw color loaded from JSON [R,G,B,A]
};
 
// ---------------------------------------------------------------------------
// Level — loads from JSON, draws itself, and exposes collision data
//
// JSON format (assets/levels/<name>.json):
//
//   {
//     "name": "Level 01",
//     "spawn": { "x": 100.0, "y": 320.0 },
//     "objects": [
//       { "x": 0, "y": 400, "width": 800, "height": 50, "color": [139,90,43,255] },
//       { "x": 150, "y": 300, "width": 200, "height": 20, "color": [100,140,60,255] }
//     ]
//   }
//
// All "objects" become both visual rectangles and collision solids.
// "color" is optional — defaults to brown if missing.
// "spawn" is optional — defaults to (100, 200) if missing.
// ---------------------------------------------------------------------------
class Level {
public:
    // Load level from a JSON file. Returns true on success, false on failure
    // (file not found, malformed JSON, missing required fields).
    // Safe to call multiple times — clears previous data first.
    bool Load(const std::string& path);
 
    // Release all data (called automatically on next Load or destruction).
    void Unload();
 
    // Draw all level objects. Call before player.Draw() so the player
    // renders on top of platforms.
    void Draw() const;
 
    // The flat list of collision rectangles — one per level object.
    // This is what Player::Collide() iterates every frame.
    const std::vector<Rectangle>& GetColliders() const { return colliders; }
 
    // Where the player should spawn when this level loads.
    Vector2 GetSpawn() const { return spawn; }
 
    // Display name loaded from JSON (useful for HUD / debug).
    const std::string& GetName() const { return name; }
 
    bool IsLoaded() const { return loaded; }
 
private:
    std::string              name    = "unnamed";
    Vector2                  spawn   = { 100.0f, 200.0f };
    std::vector<LevelObject> objects;     // drawable objects
    std::vector<Rectangle>   colliders;   // parallel flat list for fast collision queries
    bool                     loaded  = false;
};
 
extern Level level;
 
