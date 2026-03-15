#pragma once
#include <raylib.h>
#include <string>
#include <vector>

enum class LevelObjectType {
    SOLID,      // normal 4-sided collision
    ONE_SIDED,  // only top face is solid; can jump through from below
    HOSTILE,    // solid + damages player on contact
    GATE,       // solid while locked; passable when player has matching key
    HAZARD,     // NOT solid, damages player on contact (spikes etc.)
    STAIR       // auto-generated set of stepped solids
};

struct LevelObject {
    LevelObjectType type   = LevelObjectType::SOLID;
    Rectangle       bounds = {};
    Color           color  = { 139, 90, 43, 255 };
    std::string     texture;       // asset name hint for renderer
    std::string     id;            // for gates: unique ID
    std::string     keyId;         // for gates: required key
    int             damage = 0;    // for HOSTILE / HAZARD
    std::string     facing;        // for HAZARD: "up"/"down"/"left"/"right"
};

struct LevelItem {
    std::string id;     // added to inventory on pickup
    Rectangle   bounds = {};
    Color       color  = { 220, 170, 30, 255 };
    bool        collected = false;
};

// Loaded enemy spawn data — passed to EnemyManager on level load
struct EnemySpawn {
    std::string type;         // "patrol" | "jumper" | "flyer" | "thrower"
    float x = 0, y = 0;
    float patrolLeft = 0, patrolRight = 0;
    float speed = 80, jumpForce = 380, jumpInterval = 2.0f;
    float throwInterval = 3.0f, projectileSpeed = 180.0f, arcHeight = 80.0f;
    int   hp = 2, damage = 1;
    // flyer waypoints
    static constexpr int MAX_PATH = 8;
    Vector2 path[MAX_PATH];
    int     pathCount = 0;
};

class Level {
public:
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;

    // Collision geometry — separated by type for efficient queries
    const std::vector<Rectangle>& GetSolidColliders()    const { return solidColliders;    }
    const std::vector<Rectangle>& GetOneSidedColliders() const { return oneSidedColliders; }

    // Returns damage if bounds overlaps any HOSTILE or HAZARD object; else 0
    int GetContactDamage(Rectangle bounds) const;

    // Returns true if the gate with key is currently blocking bounds
    // Unlocks the gate (removes from blocking list) and records in GameData
    bool TryUnlockGate(const std::string& keyId);

    // Check and collect items; returns collected item IDs
    std::vector<std::string> CollectItems(Rectangle bounds);

    const std::vector<EnemySpawn>& GetEnemySpawns() const { return enemySpawns; }
    Vector2 GetSpawn()       const { return spawn; }
    const std::string& GetName() const { return name; }
    bool IsLoaded()          const { return loaded; }

private:
    std::string name  = "unnamed";
    Vector2     spawn = { 100.0f, 200.0f };

    std::vector<LevelObject>  objects;
    std::vector<LevelItem>    items;
    std::vector<EnemySpawn>   enemySpawns;

    // Flat collider lists built during Load for fast per-frame queries
    std::vector<Rectangle> solidColliders;
    std::vector<Rectangle> oneSidedColliders;
    std::vector<Rectangle> gateColliders;     // only unlocked gates removed

    bool loaded = false;

    void BuildColliderLists();
    // ParseObject and GenerateStairSteps are static helpers in level.cpp
};

extern Level level;
