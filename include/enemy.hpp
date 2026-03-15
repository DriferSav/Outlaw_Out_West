#pragma once
#include <raylib.h>
// level.hpp and collision.hpp are included only in enemy.cpp

enum class EnemyType { PATROL, JUMPER, FLYER, THROWER };

struct Enemy {
    EnemyType type      = EnemyType::PATROL;
    Vector2   position  = {};
    Vector2   velocity  = {};
    float     width     = 28.0f;
    float     height    = 32.0f;
    int       hp = 2, maxHp = 2;
    int       damage    = 1;
    bool      active    = false;
    bool      facingLeft = false;
    float     timer     = 0.0f; // multi-purpose per-type timer
    float     speed     = 80.0f;
    bool      onGround  = false;

    // PATROL
    float patrolLeft = 0, patrolRight = 0;

    // JUMPER
    float jumpForce = 380.0f, jumpInterval = 2.0f;

    // FLYER path
    static constexpr int MAX_PATH = 8;
    Vector2 flyPath[MAX_PATH] = {};
    int     flyPathCount = 0, flyPathIdx = 0;

    // THROWER
    float throwInterval = 3.0f, projectileSpeed = 180.0f, arcHeight = 80.0f;

    // Invincibility after hit
    float invTimer = 0.0f;
    static constexpr float INV_DURATION = 0.3f;

    Rectangle Bounds() const { return { position.x, position.y, width, height }; }
};

class EnemyManager {
public:
    static constexpr int MAX_ENEMIES = 64;

    void Clear();
    void SpawnFromLevel();   // reads level.GetEnemySpawns()
    void Update(float dt);
    void Draw() const;

    // Returns contact damage if any active enemy overlaps bounds; 0 otherwise
    int  GetContactDamage(Rectangle bounds) const;

    // Damage the first enemy whose bounds contains pos, by amount.
    // Returns true if an enemy was hit.
    bool HitAt(Vector2 pos, float radius, int amount);

private:
    Enemy enemies[MAX_ENEMIES];
    int   count = 0;

    void UpdatePatrol  (Enemy& e, float dt);
    void UpdateJumper  (Enemy& e, float dt);
    void UpdateFlyer   (Enemy& e, float dt);
    void UpdateThrower (Enemy& e, float dt);
    void ApplyGravity  (Enemy& e, float dt);
    void ApplyCollision(Enemy& e);
};

extern EnemyManager enemyManager;
