#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <player.hpp>

Player::Player() {
    position = { 100.0f, 300.0f };
    velocity = { 0.0f, 0.0f };
}

void Player::Update(float deltaTime) {
    HandleInput();

    ApplyGravity(deltaTime);

    Move(deltaTime);
    Collide();
}

void Player::Draw() {
    DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(width), static_cast<int>(height), RED);
}

void Player::HandleInput() {
    float vx = 0.0f;
    if(input.IsHeld(Action::MOVE_RIGHT)) vx = moveSpeed;
    else if(input.IsHeld(Action::MOVE_LEFT)) vx = -moveSpeed;

    velocity.x = vx;

    if(input.IsPressed(Action::JUMP) && onGround) {
        velocity.y = -jumpForce;
        onGround = false;
    }
}

void Player::ApplyGravity(float deltaTime) {
    velocity.y += gravity * deltaTime;
    if(velocity.y > fallSpeed) velocity.y = fallSpeed;
}

void Player::Move(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}

void Player::Collide() {
    float groundY = 300.0f;
    if (position.y + height >= groundY) {
        position.y = groundY - height;
        velocity.y = 0.0f;
        onGround = true;
    } else {
        onGround = false;
    }
}

Player player;
