#include <enemy.hpp>
#include <bullet.hpp>
#include <level.hpp>
#include <collision.hpp>
#include <globals.hpp>
#include <cmath>
#include <algorithm>

EnemyManager enemyManager;

void EnemyManager::Clear() {
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    count = 0;
}

void EnemyManager::SpawnFromLevel() {
    Clear();
    for (const auto& sp : level.GetEnemySpawns()) {
        if (count >= MAX_ENEMIES) break;
        Enemy& e     = enemies[count++];
        e.active     = true;
        e.position   = { sp.x, sp.y };
        e.velocity   = {};
        e.hp = e.maxHp = sp.hp;
        e.damage     = sp.damage;
        e.speed      = sp.speed;
        e.patrolLeft = sp.patrolLeft;
        e.patrolRight= sp.patrolRight;
        e.jumpForce  = sp.jumpForce;
        e.jumpInterval   = sp.jumpInterval;
        e.throwInterval  = sp.throwInterval;
        e.projectileSpeed= sp.projectileSpeed;
        e.arcHeight      = sp.arcHeight;
        e.timer = 0.0f;
        if      (sp.type == "patrol")  e.type = EnemyType::PATROL;
        else if (sp.type == "jumper")  e.type = EnemyType::JUMPER;
        else if (sp.type == "flyer")   e.type = EnemyType::FLYER;
        else if (sp.type == "thrower") e.type = EnemyType::THROWER;
        for (int i = 0; i < sp.pathCount && i < Enemy::MAX_PATH; i++)
            e.flyPath[i] = sp.path[i];
        e.flyPathCount = sp.pathCount;
        e.flyPathIdx   = 0;
    }
}

void EnemyManager::ApplyGravity(Enemy& e, float dt) {
    if (!e.onGround) {
        e.velocity.y += 1200.0f * dt;
        if (e.velocity.y > 800.0f) e.velocity.y = 800.0f;
    }
}

void EnemyManager::ApplyCollision(Enemy& e) {
    bool dummy1=false, dummy2=false;
    e.onGround = false;
    if (level.IsLoaded())
        ResolveAll(e.position, e.velocity, e.width, e.height,
                   level.GetSolidColliders(), e.onGround, dummy1, dummy2);
}

void EnemyManager::UpdatePatrol(Enemy& e, float dt) {
    e.velocity.x = e.facingLeft ? -e.speed : e.speed;
    e.position.x += e.velocity.x * dt;
    if (e.position.x <= e.patrolLeft)  { e.position.x = e.patrolLeft;  e.facingLeft = false; }
    if (e.position.x + e.width >= e.patrolRight) { e.position.x = e.patrolRight - e.width; e.facingLeft = true; }
    ApplyGravity(e, dt);
    e.position.y += e.velocity.y * dt;
    ApplyCollision(e);
}

void EnemyManager::UpdateJumper(Enemy& e, float dt) {
    e.timer += dt;
    if (e.onGround && e.timer >= e.jumpInterval) {
        e.velocity.y = -e.jumpForce;
        e.timer = 0.0f;
    }
    ApplyGravity(e, dt);
    e.position.y += e.velocity.y * dt;
    ApplyCollision(e);
}

void EnemyManager::UpdateFlyer(Enemy& e, float dt) {
    if (e.flyPathCount < 2) return;
    Vector2 target = e.flyPath[e.flyPathIdx];
    float dx = target.x - e.position.x;
    float dy = target.y - e.position.y;
    float dist = sqrtf(dx*dx + dy*dy);
    if (dist < 4.0f) {
        e.flyPathIdx = (e.flyPathIdx + 1) % e.flyPathCount;
    } else {
        e.position.x += (dx / dist) * e.speed * dt;
        e.position.y += (dy / dist) * e.speed * dt;
        e.facingLeft = dx < 0;
    }
}

void EnemyManager::UpdateThrower(Enemy& e, float dt) {
    e.timer += dt;
    ApplyGravity(e, dt);
    e.position.y += e.velocity.y * dt;
    ApplyCollision(e);
    if (e.onGround && e.timer >= e.throwInterval) {
        e.timer = 0.0f;
        // Lob a bullet in an arc toward the player's general direction
        // The arc is simulated by giving a vertical component
        float dir    = e.facingLeft ? -1.0f : 1.0f;
        float vx     = dir * e.projectileSpeed;
        float vy     = -sqrtf(2.0f * 1200.0f * e.arcHeight); // vy to reach arcHeight
        Vector2 tip  = { e.position.x + e.width * 0.5f, e.position.y };
        float angle  = atan2f(vy, vx);
        bulletPool.SpawnEnemy(tip, angle, e.projectileSpeed);
    }
}

void EnemyManager::Update(float dt) {
    for (int i = 0; i < count; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;
        if (e.invTimer > 0.0f) e.invTimer -= dt;
        switch (e.type) {
            case EnemyType::PATROL:  UpdatePatrol (e, dt); break;
            case EnemyType::JUMPER:  UpdateJumper (e, dt); break;
            case EnemyType::FLYER:   UpdateFlyer  (e, dt); break;
            case EnemyType::THROWER: UpdateThrower(e, dt); break;
        }
    }
}

void EnemyManager::Draw() const {
    for (int i = 0; i < count; i++) {
        const Enemy& e = enemies[i];
        if (!e.active) continue;
        Color c = (e.invTimer > 0.0f) ? Color{255,255,255,180} : Color{60,180,60,255};
        DrawRectangleRec(e.Bounds(), c);
        // Health bar
        if (e.hp < e.maxHp) {
            float pct = (float)e.hp / e.maxHp;
            DrawRectangle((int)e.position.x, (int)e.position.y - 6, (int)e.width, 3, DARKGRAY);
            DrawRectangle((int)e.position.x, (int)e.position.y - 6, (int)(e.width * pct), 3, GREEN);
        }
    }
}

int EnemyManager::GetContactDamage(Rectangle bounds) const {
    for (int i = 0; i < count; i++) {
        const Enemy& e = enemies[i];
        if (!e.active) continue;
        if (CheckCollisionRecs(bounds, e.Bounds())) return e.damage;
    }
    return 0;
}

bool EnemyManager::HitAt(Vector2 pos, float radius, int amount) {
    for (int i = 0; i < count; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;
        if (e.invTimer > 0.0f) continue;
        if (CheckCollisionCircleRec(pos, radius, e.Bounds())) {
            e.hp -= amount;
            e.invTimer = Enemy::INV_DURATION;
            if (e.hp <= 0) e.active = false;
            return true;
        }
    }
    return false;
}
