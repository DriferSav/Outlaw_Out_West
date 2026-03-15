#include <bullet.hpp>
#include <enemy.hpp>
#include <level.hpp>
#include <globals.hpp>
#include <cmath>

BulletPool bulletPool;

int BulletPool::FindFree() const {
    for (int i = 0; i < MAX_BULLETS; i++) if (!bullets[i].active) return i;
    return -1;
}

void BulletPool::Spawn(Vector2 pos, float angle) {
    int idx = FindFree(); if (idx < 0) return;
    Bullet& b = bullets[idx];
    b.position  = pos;
    b.velocity  = { cosf(angle)*BULLET_SPEED, sinf(angle)*BULLET_SPEED };
    b.timeLeft  = BULLET_LIFETIME;
    b.active    = true;
    b.fromEnemy = false;
}

void BulletPool::SpawnEnemy(Vector2 pos, float angle, float spd) {
    int idx = FindFree(); if (idx < 0) return;
    Bullet& b = bullets[idx];
    b.position  = pos;
    b.velocity  = { cosf(angle)*spd, sinf(angle)*spd };
    b.timeLeft  = BULLET_LIFETIME;
    b.active    = true;
    b.fromEnemy = true;
}

bool BulletPool::Update(float dt, Rectangle playerBounds, int& enemyBulletsHit) {
    enemyBulletsHit = 0;
    bool anyEnemyHit = false;

    for (Bullet& b : bullets) {
        if (!b.active) continue;
        b.position.x += b.velocity.x * dt;
        b.position.y += b.velocity.y * dt;
        b.timeLeft   -= dt;

        if (b.timeLeft <= 0.0f) { b.active = false; continue; }

        // Screen bounds
        if (b.position.x < -BULLET_RADIUS || b.position.x > Global::SCREEN_WIDTH  + BULLET_RADIUS ||
            b.position.y < -BULLET_RADIUS || b.position.y > Global::SCREEN_HEIGHT + BULLET_RADIUS)
        { b.active = false; continue; }

        // Level geometry
        if (level.IsLoaded()) {
            bool hit = false;
            for (const Rectangle& r : level.GetSolidColliders())
                if (CheckCollisionCircleRec(b.position, BULLET_RADIUS, r)) { hit = true; break; }
            if (hit) { b.active = false; continue; }
        }

        if (!b.fromEnemy) {
            // Player bullet — check enemies
            if (enemyManager.HitAt(b.position, BULLET_RADIUS, 1)) {
                b.active = false;
                anyEnemyHit = true;
            }
        } else {
            // Enemy bullet — check player
            if (CheckCollisionCircleRec(b.position, BULLET_RADIUS, playerBounds)) {
                b.active = false;
                enemyBulletsHit++;
            }
        }
    }
    return anyEnemyHit;
}

void BulletPool::Draw() const {
    for (const Bullet& b : bullets) {
        if (!b.active) continue;
        Color c = b.fromEnemy ? Color{255,100,50,255} : Color{255,220,50,255};
        DrawCircleV(b.position, BULLET_RADIUS, c);
        DrawCircleV(b.position, BULLET_RADIUS - 1.5f, {255,255,200,255});
    }
}

void BulletPool::Clear() { for (Bullet& b : bullets) b.active = false; }

int BulletPool::ActiveCount() const {
    int n = 0; for (const Bullet& b : bullets) if (b.active) n++; return n;
}
