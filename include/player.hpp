#pragma once
#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
 
enum class PlayerMovementState  { IDLE, WALK, RUN, JUMP };
enum class PlayerEnvironmentState { ON_GROUND, RISING_IN_AIR, FALLING_IN_AIR };
enum  PlayerInteractionState    { INTERACT, ATTACK };
 
class Player {
public:
    Player();
 
    void Update(float deltaTime);
    void Draw() const;
 
    // For FrameHistory snapshot (called in main.cpp before Update)
    PlayerFrame GetFrame() const;
 
    // Public accessors used by BulletPool spawn
    Vector2 GunBarrelTip() const;
    float   GetAimAngle()  const { return aimAngle; }
 
private:
    Vector2 position     = {};
    Vector2 velocity     = {};
 
    float width          = 32.0f;
    float height         = 48.0f;
    float moveSpeed      = 220.0f;
    float jumpForce      = 420.0f;
    float gravity        = 1200.0f;
    float fallSpeed      = 900.0f;
 
    float aimAngle       = 0.0f;   // radians: 0 = right, π = left, ±π/2 = up/down
    bool  facingLeft     = false;  // true when cos(aimAngle) < 0 — for sprite flipping later
    float shootCooldown  = 0.0f;   // seconds until next shot allowed
 
    bool onGround        = false;
 
    static constexpr int   COYOTE_FRAMES      = 8;
    static constexpr int   JUMP_BUFFER_FRAMES = 10;
    static constexpr float SHOOT_COOLDOWN     = 0.25f; // 4 shots/sec max
    static constexpr float GUN_LENGTH         = 28.0f; // px from player center to barrel tip
    static constexpr float GUN_THICKNESS      = 5.0f;
 
    void UpdateAim();
    void HandleInput();
    void ApplyGravity(float deltaTime);
    void Move(float deltaTime);
    void Collide();
 
    bool WasRecentlyGrounded() const;
    bool HasBufferedJump()     const;
};
 
extern Player player;
 
