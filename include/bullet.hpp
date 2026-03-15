#pragma once
#include <raylib.h>

struct Bullet {
    Vector2 position  = {};
    Vector2 velocity  = {};
    float   timeLeft  = 0.0f;
    bool    active    = false;
    bool    fromEnemy = false; // enemy bullets check player; player bullets check enemies
};

class BulletPool {
public:
    static constexpr int   MAX_BULLETS    = 64;
    static constexpr float BULLET_SPEED   = 650.0f;
    static constexpr float BULLET_LIFETIME= 1.8f;
    static constexpr float BULLET_RADIUS  = 4.0f;

    void Spawn     (Vector2 pos, float angle);            // player bullet
    void SpawnEnemy(Vector2 pos, float angle, float spd); // enemy bullet (arc already baked into angle)

    // Returns true if any player bullet hit an enemy (removes bullet + damages enemy)
    // Returns true if any enemy bullet hit the player rect (removes bullet, caller handles damage)
    bool Update(float dt, Rectangle playerBounds, int& enemyBulletsHit);

    void Draw()   const;
    void Clear();
    int  ActiveCount() const;

private:
    Bullet bullets[MAX_BULLETS];
    int    FindFree() const;
};

extern BulletPool bulletPool;
