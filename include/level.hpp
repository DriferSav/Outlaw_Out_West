#pragma once
#include <raylib.h>
#include <string>
#include <vector>

enum class LevelObjectType {
    SOLID,      // normal 4-sided collision
    ONE_SIDED,  // only top face is solid; jump through from below, hold DOWN to fall through
    GATE,       // solid while locked; requires interact + key to open/win
    HAZARD,     // NOT solid, damages player on contact
};

struct LevelObject {
    LevelObjectType type   = LevelObjectType::SOLID;
    Rectangle       bounds = {};
    Color           color  = { 139, 90, 43, 255 };
    std::string     texture;
    std::string     id;       // unique gate ID
    std::string     keyId;    // required key ID
    int             damage  = 0;
    std::string     facing;   // for HAZARD orientation
    bool            isExit  = false; // if true, interacting triggers WIN
};

struct LevelItem {
    std::string id;
    Rectangle   bounds    = {};
    Color       color     = { 220, 170, 30, 255 };
    bool        collected = false;
};

struct EnemySpawn {
    std::string type;
    float x = 0, y = 0;
    float patrolLeft = 0, patrolRight = 0;
    float speed = 80, jumpForce = 380, jumpInterval = 2.0f;
    float throwInterval = 3.0f, projectileSpeed = 180.0f, arcHeight = 80.0f;
    int   hp = 2, damage = 1;
    static constexpr int MAX_PATH = 8;
    Vector2 path[MAX_PATH];
    int     pathCount = 0;
};

class Level {
public:
    bool Load(const std::string& path);
    void Unload();
    void Draw() const;

    const std::vector<Rectangle>& GetSolidColliders()    const { return solidColliders;    }
    const std::vector<Rectangle>& GetOneSidedColliders() const { return oneSidedColliders; }

    // Returns damage value if bounds overlaps any HAZARD; else 0
    int GetContactDamage(Rectangle bounds) const;

    // Collect key items the player is touching; returns their IDs
    std::vector<std::string> CollectItems(Rectangle bounds);

    // Try to interact with an exit gate:
    //   - player bounds must overlap the gate
    //   - inventory must contain the gate's keyId
    // Returns true (win!) on success. Does NOT modify gate state.
    bool TryInteractGate(Rectangle playerBounds,
                         const std::vector<std::string>& inventory) const;

    // Unlock a non-exit gate with a key (removes from solid colliders).
    // Called when a regular (non-exit) key is collected.
    bool TryUnlockGate(const std::string& keyId);

    const std::vector<EnemySpawn>& GetEnemySpawns() const { return enemySpawns; }
    Vector2            GetSpawn()  const { return spawn;  }
    Vector2            GetWorldSize() const { return worldSize; } // world pixel dimensions
    const std::string& GetName()   const { return name;   }
    bool               IsLoaded()  const { return loaded; }

private:
    std::string name  = "unnamed";
    Vector2     spawn  = { 160.0f, 480.0f };
    Vector2     worldSize = { 99999.0f, 99999.0f }; // safe until Load() parses "bounds"

    std::vector<LevelObject>  objects;
    std::vector<LevelItem>    items;
    std::vector<EnemySpawn>   enemySpawns;

    std::vector<Rectangle> solidColliders;
    std::vector<Rectangle> oneSidedColliders;
    std::vector<Rectangle> gateColliders;

    bool loaded = false;

    void BuildColliderLists();
    // ParseObjectInto is a static helper in level.cpp
};

extern Level level;
