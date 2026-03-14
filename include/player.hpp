#pragma once
#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>

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

    private:
    
    Vector2 position{};
    Vector2 velocity{};

    float width = 32.0f;
    float height = 48.0f;

    float moveSpeed = 220.0f;
    float jumpForce = 420.0f;
    float gravity = 1200.0f;
    float fallSpeed = 900.0f;
    
    bool onGround = false;
    bool risingInAir = false;
    bool fallingInAir = false;

    void HandleInput();
    void ApplyGravity(float deltaTime);
    void Move(float deltaTime);
    void Collide();
};

extern Player player;
