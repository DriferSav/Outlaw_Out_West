#pragma once
#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
 
enum class PlayerMovementState {
    IDLE,
    WALK,
    RUN,
    JUMP,
};
 
enum class PlayerEnvironmentState {
    ON_GROUND,
    RISING_IN_AIR,
    FALLING_IN_AIR
};
 
enum PlayerInteractionState {
    INTERACT,
    ATTACK
};
 
class Player {
public:
    Player();
 
    void Update(float deltaTime);
    void Draw();
 
    // Snapshot current physics state for archiving into FrameHistory.
    // Called in main.cpp BEFORE Update() so the snapshot reflects
    // the state at the START of this tick.
    PlayerFrame GetFrame() const;
 
private:
    Vector2 position = {};
    Vector2 velocity = {};
 
    float width     = 32.0f;
    float height    = 48.0f;
    float moveSpeed = 220.0f;
    float jumpForce = 420.0f;
    float gravity   = 1200.0f;
    float fallSpeed = 900.0f;
 
    bool onGround = false;
 
    // Frames after leaving ground that a late jump is still granted.
    // 8 frames = ~133ms at 60 fps — standard platformer coyote window.
    static constexpr int COYOTE_FRAMES = 8;
 
    // Frames before landing that a jump press is remembered and auto-fires.
    // 10 frames = ~166ms at 60 fps.
    static constexpr int JUMP_BUFFER_FRAMES = 10;
 
    void HandleInput();
    void ApplyGravity(float deltaTime);
    void Move(float deltaTime);
    void Collide();
 
    // Looks back through FrameHistory: were we grounded in the last COYOTE_FRAMES?
    bool WasRecentlyGrounded() const;
 
    // Looks back through FrameHistory: did JUMP rise within JUMP_BUFFER_FRAMES?
    bool HasBufferedJump() const;
};
 
extern Player player;
 
