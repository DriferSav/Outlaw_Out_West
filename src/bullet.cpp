#include <bullet.hpp>
#include <level.hpp>
#include <globals.hpp>
#include <cmath>
 
BulletPool bulletPool;
 
int BulletPool::FindFree() const {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) return i;
    }
    return -1; // pool full — caller drops the shot
}
 
void BulletPool::Spawn(Vector2 pos, float angle) {
    const int idx = FindFree();
    if (idx < 0) return; // pool exhausted, drop silently
 
    Bullet& b    = bullets[idx];
    b.position   = pos;
    b.velocity   = { cosf(angle) * BULLET_SPEED,
                     sinf(angle) * BULLET_SPEED };
    b.timeLeft   = BULLET_LIFETIME;
    b.active     = true;
}
 
void BulletPool::Update(float deltaTime) {
    for (Bullet& b : bullets) {
        if (!b.active) continue;
 
        // Move
        b.position.x += b.velocity.x * deltaTime;
        b.position.y += b.velocity.y * deltaTime;
        b.timeLeft   -= deltaTime;
 
        // Despawn: lifetime expired
        if (b.timeLeft <= 0.0f) { b.active = false; continue; }
 
        // Despawn: left the screen
        if (b.position.x < -BULLET_RADIUS ||
            b.position.x > Global::SCREEN_WIDTH  + BULLET_RADIUS ||
            b.position.y < -BULLET_RADIUS ||
            b.position.y > Global::SCREEN_HEIGHT + BULLET_RADIUS)
        {
            b.active = false;
            continue;
        }
 
        // Despawn: hit a solid platform
        if (level.IsLoaded()) {
            for (const Rectangle& r : level.GetColliders()) {
                if (CheckCollisionCircleRec(b.position, BULLET_RADIUS, r)) {
                    b.active = false;
                    break;
                }
            }
        }
    }
}
 
void BulletPool::Draw() const {
    for (const Bullet& b : bullets) {
        if (!b.active) continue;
 
        // Draw as a bright yellow circle with a slightly darker outline
        DrawCircleV(b.position, BULLET_RADIUS,     { 255, 220,  50, 255 }); // yellow core
        DrawCircleV(b.position, BULLET_RADIUS - 1, { 255, 255, 160, 255 }); // bright center
    }
}
 
void BulletPool::Clear() {
    for (Bullet& b : bullets) b.active = false;
}
 
int BulletPool::ActiveCount() const {
    int n = 0;
    for (const Bullet& b : bullets) if (b.active) n++;
    return n;
}
 
