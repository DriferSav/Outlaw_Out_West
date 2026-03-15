#pragma once
#include <raylib.h>
 
// ---------------------------------------------------------------------------
// Bullet — one projectile slot in the pool
// ---------------------------------------------------------------------------
struct Bullet {
    Vector2 position  = {};
    Vector2 velocity  = {};
    float   timeLeft  = 0.0f; // seconds until auto-despawn
    bool    active    = false;
};
 
// ---------------------------------------------------------------------------
// BulletPool — fixed-size object pool; no heap allocation after startup
//
// Usage:
//   bulletPool.Spawn(player.GunBarrelTip(), player.GetAimAngle());
//   bulletPool.Update(dt);   // in game update
//   bulletPool.Draw();        // in game draw
// ---------------------------------------------------------------------------
class BulletPool {
public:
    static constexpr int   MAX_BULLETS    = 32;
    static constexpr float BULLET_SPEED   = 650.0f;  // px/s
    static constexpr float BULLET_LIFETIME= 1.8f;    // seconds
    static constexpr float BULLET_RADIUS  = 4.0f;    // for collision + draw
 
    // Fire a bullet from pos in direction angle (radians).
    // Silently drops if pool is full.
    void Spawn(Vector2 pos, float angle);
 
    // Move all active bullets; deactivate on timeout or level collision.
    void Update(float deltaTime);
 
    // Draw all active bullets.
    void Draw() const;
 
    // Kill all bullets (call on level reload).
    void Clear();
 
    int ActiveCount() const;
 
private:
    Bullet bullets[MAX_BULLETS];
 
    // Returns index of first inactive slot, or -1 if pool full.
    int FindFree() const;
};
 
extern BulletPool bulletPool;
 
