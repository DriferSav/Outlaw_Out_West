#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <collision.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <player.hpp>
#include <cmath>
 
Player::Player() {
    position = { 100.0f, 300.0f };
    velocity = { 0.0f, 0.0f };
}
 
PlayerFrame Player::GetFrame() const {
    return { position, velocity, aimAngle, onGround };
}
 
// ---------------------------------------------------------------------------
// Geometry helpers
// ---------------------------------------------------------------------------
 
// The gun pivots at chest height (center of body, shifted up slightly).
static Vector2 ChestCenter(Vector2 pos, float w, float h) {
    return { pos.x + w * 0.5f, pos.y + h * 0.5f - 4.0f };
}
 
Vector2 Player::GunBarrelTip() const {
    const Vector2 chest = ChestCenter(position, width, height);
    return {
        chest.x + cosf(aimAngle) * GUN_LENGTH,
        chest.y + sinf(aimAngle) * GUN_LENGTH
    };
}
 
// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------
void Player::Update(float deltaTime) {
    // Tick timers before HandleInput so cooldown is current this frame
    if (shootCooldown > 0.0f) shootCooldown -= deltaTime;
 
    UpdateAim();
    HandleInput();
    ApplyGravity(deltaTime);
    Move(deltaTime);
    Collide();
}
 
// ---------------------------------------------------------------------------
// Aim — mouse or right joystick, joystick wins when pushed past deadzone
// ---------------------------------------------------------------------------
void Player::UpdateAim() {
    const Vector2 stick = input.GetRightStick();
    const Vector2 chest = ChestCenter(position, width, height);
 
    if (stick.x != 0.0f || stick.y != 0.0f) {
        // Joystick: angle comes directly from stick axes
        aimAngle = atan2f(stick.y, stick.x);
    } else {
        // Mouse: angle from chest center to cursor position
        const Vector2 mouse = GetMousePosition();
        aimAngle = atan2f(mouse.y - chest.y, mouse.x - chest.x);
    }
 
    // Facing direction follows horizontal component of aim angle
    facingLeft = (cosf(aimAngle) < 0.0f);
}
 
// ---------------------------------------------------------------------------
// Input — movement, jump, shoot
// ---------------------------------------------------------------------------
void Player::HandleInput() {
    // Horizontal movement
    float vx = 0.0f;
    if (input.IsHeld(Action::MOVE_RIGHT)) vx =  moveSpeed;
    if (input.IsHeld(Action::MOVE_LEFT))  vx = -moveSpeed;
    velocity.x = vx;
 
    // Jump — normal, coyote, buffered (priority order)
    const bool normalJump   = onGround && input.IsPressed(Action::JUMP);
    const bool coyoteJump   = !onGround && input.IsPressed(Action::JUMP) && WasRecentlyGrounded();
    const bool bufferedJump = onGround && !normalJump && HasBufferedJump();
    if (normalJump || coyoteJump || bufferedJump) {
        velocity.y = -jumpForce;
        onGround   = false;
    }
 
    // Shoot — mouse left click OR gamepad right trigger
    const bool mouseShoot    = input.IsPressed(Action::SHOOT);
    const bool triggerShoot  = IsGamepadAvailable(0) &&
                               GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) > 0.5f;
 
    if ((mouseShoot || triggerShoot) && shootCooldown <= 0.0f) {
        bulletPool.Spawn(GunBarrelTip(), aimAngle);
        shootCooldown = SHOOT_COOLDOWN;
    }
}
 
// ---------------------------------------------------------------------------
// Physics
// ---------------------------------------------------------------------------
void Player::ApplyGravity(float deltaTime) {
    if (!onGround) {
        velocity.y += gravity * deltaTime;
        if (velocity.y > fallSpeed) velocity.y = fallSpeed;
    }
}
 
void Player::Move(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}
 
void Player::Collide() {
    onGround = false; // reset every frame — ResolveAll sets it true on landing
 
    if (level.IsLoaded()) {
        ResolveAll(position, velocity, width, height, level.GetColliders(), onGround);
    }
 
    // Screen-edge clamp
    if (position.x < 0.0f) { position.x = 0.0f; velocity.x = 0.0f; }
    const float rightEdge = static_cast<float>(Global::SCREEN_WIDTH) - width;
    if (position.x > rightEdge) { position.x = rightEdge; velocity.x = 0.0f; }
}
 
// ---------------------------------------------------------------------------
// Draw — body + gun arm
// ---------------------------------------------------------------------------
void Player::Draw() const {
    const Vector2 chest = ChestCenter(position, width, height);
    const Vector2 tip   = GunBarrelTip();
 
    // Body — slightly lighter on the side facing the aim direction
    const Color bodyColor = { 210, 50, 50, 255 };
    DrawRectangle(
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        static_cast<int>(width),
        static_cast<int>(height),
        bodyColor);
 
    // Hat brim — tiny visual hint this is a cowboy
    const int hatX = static_cast<int>(position.x) - 4;
    const int hatY = static_cast<int>(position.y) - 6;
    DrawRectangle(hatX,     hatY + 2, static_cast<int>(width) + 8, 4, DARKBROWN); // brim
    DrawRectangle(hatX + 6, hatY - 8, static_cast<int>(width) - 4, 10, DARKBROWN); // crown
 
    // Gun arm — line from chest pivot to barrel tip, then muzzle dot
    DrawLineEx(chest, tip, GUN_THICKNESS, DARKGRAY);
    DrawCircleV(tip, 3.5f, GRAY);
 
    // Muzzle flash: tiny circle that appears only while cooldown is near-fresh
    if (shootCooldown > SHOOT_COOLDOWN * 0.6f) {
        DrawCircleV(tip, 6.0f, { 255, 220, 80, 200 });
    }
}
 
// ---------------------------------------------------------------------------
// Frame history queries
// ---------------------------------------------------------------------------
bool Player::WasRecentlyGrounded() const {
    for (int i = 1; i <= COYOTE_FRAMES; i++) {
        const FrameState* f = frameHistory.Get(static_cast<size_t>(i));
        if (!f) break;
        if (f->player.onGround) return true;
    }
    return false;
}
 
bool Player::HasBufferedJump() const {
    for (int i = 0; i < JUMP_BUFFER_FRAMES; i++) {
        const FrameState* cur  = frameHistory.Get(static_cast<size_t>(i));
        const FrameState* prev = frameHistory.Get(static_cast<size_t>(i + 1));
        if (!cur || !prev) break;
        if (cur->input.IsHeld(Action::JUMP) && !prev->input.IsHeld(Action::JUMP)) return true;
    }
    return false;
}
 
Player player;
 
