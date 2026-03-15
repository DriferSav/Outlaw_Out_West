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
// Geometry
// ---------------------------------------------------------------------------
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
    if (isDead) return; // freeze everything once dead

    if (shootCooldown > 0.0f) shootCooldown -= deltaTime;

    UpdateAim();
    HandleInput();
    ApplyGravity(deltaTime);
    Move(deltaTime);
    Collide();
    UpdateStates(); // must run after Collide so onGround is current
}

// ---------------------------------------------------------------------------
// Aim — fix: gun angle persists when gamepad stick returns to center
//
// Priority rules:
//   1. If mouse moved this frame   → switch to MOUSE, update angle from cursor
//   2. If right stick past deadzone → switch to GAMEPAD, update angle from stick
//   3. Neither moved               → keep aimAngle exactly as it was
//
// This means a gamepad player who stops aiming keeps the last aimed direction.
// A mouse player who picks up the stick has instant seamless takeover.
// ---------------------------------------------------------------------------
void Player::UpdateAim() {
    const Vector2 stick      = input.GetRightStick();
    const Vector2 mouseDelta = GetMouseDelta();
    const bool    stickActive = (stick.x != 0.0f || stick.y != 0.0f);
    const bool    mouseMoved  = (fabsf(mouseDelta.x) > 0.5f || fabsf(mouseDelta.y) > 0.5f);

    if (stickActive) {
        // Gamepad stick pushed — use it and remember
        aimAngle      = atan2f(stick.y, stick.x);
        lastAimDevice = AimDevice::GAMEPAD;
    } else if (mouseMoved) {
        // Mouse cursor moved — switch to mouse
        lastAimDevice = AimDevice::MOUSE;
    }

    // Only update from mouse when mouse is the active device
    if (lastAimDevice == AimDevice::MOUSE) {
        const Vector2 chest = ChestCenter(position, width, height);
        const Vector2 mouse = GetMousePosition();
        aimAngle = atan2f(mouse.y - chest.y, mouse.x - chest.x);
    }
    // If lastAimDevice == GAMEPAD and stick is not active: aimAngle unchanged

    facingLeft = (cosf(aimAngle) < 0.0f);
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void Player::HandleInput() {
    // ---- Horizontal movement ----
    // Digital (keyboard / D-pad) and analog (left stick) — larger magnitude wins
    float vx = 0.0f;
    if (input.IsHeld(Action::MOVE_RIGHT)) vx =  moveSpeed;
    if (input.IsHeld(Action::MOVE_LEFT))  vx = -moveSpeed;

    const Vector2 ls = input.GetLeftStick();
    if (fabsf(ls.x) * moveSpeed > fabsf(vx))
        vx = ls.x * moveSpeed;

    velocity.x = vx;

    // ---- Jump cut (check release BEFORE new press) ----
    if (jumpHeld && velocity.y < 0.0f && input.IsReleased(Action::JUMP)) {
        velocity.y *= JUMP_CUT_FACTOR;
        jumpHeld = false;
    }
    if (velocity.y >= 0.0f) jumpHeld = false; // clear at apex

    // ---- Jump launch ----
    const bool normalJump   = onGround && input.IsPressed(Action::JUMP);
    const bool coyoteJump   = !onGround && input.IsPressed(Action::JUMP)
                                        && WasRecentlyGrounded();
    const bool bufferedJump = onGround && !normalJump && HasBufferedJump();

    if (normalJump || coyoteJump || bufferedJump) {
        velocity.y = -JUMP_FORCE;
        jumpHeld   = true;
        onGround   = false;
    }

    // ---- Shoot ----
    const bool buttonShoot  = input.IsPressed(Action::SHOOT);
    const bool triggerShoot = input.GetRightTrigger() > 0.5f;

    if ((buttonShoot || triggerShoot) && shootCooldown <= 0.0f) {
        bulletPool.Spawn(GunBarrelTip(), aimAngle);
        shootCooldown = SHOOT_COOLDOWN;
    }
}

// ---------------------------------------------------------------------------
// Gravity
// ---------------------------------------------------------------------------
void Player::ApplyGravity(float deltaTime) {
    if (onGround) return;

    float grav;
    if      (velocity.y < 0.0f && jumpHeld) grav = GRAVITY_RISING;
    else if (velocity.y > 0.0f)             grav = GRAVITY_FALLING;
    else                                    grav = GRAVITY_NORMAL;

    velocity.y += grav * deltaTime;
    if (velocity.y > fallSpeed) velocity.y = fallSpeed;
}

void Player::Move(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}

// ---------------------------------------------------------------------------
// Collide
// ---------------------------------------------------------------------------
void Player::Collide() {
    onGround = false;

    if (level.IsLoaded())
        ResolveAll(position, velocity, width, height, level.GetColliders(), onGround);

    // Screen edges
    if (position.x < 0.0f) { position.x = 0.0f; velocity.x = 0.0f; }
    const float rightEdge = static_cast<float>(Global::SCREEN_WIDTH) - width;
    if (position.x > rightEdge) { position.x = rightEdge; velocity.x = 0.0f; }

    // Death: fell off the bottom of the screen
    if (position.y > static_cast<float>(Global::SCREEN_HEIGHT) + 100.0f)
        isDead = true;
}

// ---------------------------------------------------------------------------
// UpdateStates — derives all three enums from current physics values
// Call after Collide() so onGround reflects this frame's resolution.
// ---------------------------------------------------------------------------
void Player::UpdateStates() {
    // ---- Environment ----
    if (onGround) {
        environmentState = PlayerEnvironmentState::ON_GROUND;
    } else if (velocity.y < 0.0f) {
        environmentState = PlayerEnvironmentState::RISING_IN_AIR;
    } else {
        environmentState = PlayerEnvironmentState::FALLING_IN_AIR;
    }

    // ---- Movement ----
    // JUMP takes priority over horizontal state when airborne.
    if (environmentState != PlayerEnvironmentState::ON_GROUND) {
        movementState = PlayerMovementState::JUMP;
    } else if (fabsf(velocity.x) > moveSpeed * 0.8f) {
        movementState = PlayerMovementState::RUN;
    } else if (fabsf(velocity.x) > 2.0f) {
        movementState = PlayerMovementState::WALK;
    } else {
        movementState = PlayerMovementState::IDLE;
    }

    // ---- Interaction ----
    // ATTACK while the muzzle flash is visible; IDLE otherwise
    interactionState = (shootCooldown > SHOOT_COOLDOWN * 0.6f)
        ? PlayerInteractionState::ATTACK
        : PlayerInteractionState::IDLE;
}

// ---------------------------------------------------------------------------
// Draw — uses state to tint body and hint animation
// ---------------------------------------------------------------------------
void Player::Draw() const {
    const Vector2 chest = ChestCenter(position, width, height);
    const Vector2 tip   = GunBarrelTip();

    // Body color varies with movement state
    Color bodyColor;
    switch (movementState) {
        case PlayerMovementState::IDLE: bodyColor = {  180,  40,  40, 255 }; break; // darker red — still
        case PlayerMovementState::WALK: bodyColor = {  210,  55,  55, 255 }; break; // mid red
        case PlayerMovementState::RUN:  bodyColor = {  240,  70,  50, 255 }; break; // orange-red — moving fast
        case PlayerMovementState::JUMP: bodyColor = {  200,  60, 180, 255 }; break; // purple tint — airborne
    }
    // Flash white briefly when attacking
    if (interactionState == PlayerInteractionState::ATTACK)
        bodyColor = { 255, 220, 180, 255 };

    DrawRectangle(
        static_cast<int>(position.x), static_cast<int>(position.y),
        static_cast<int>(width),      static_cast<int>(height),
        bodyColor);

    // Hat — brim shifts right when running right, left when running left
    const int lean  = (movementState == PlayerMovementState::RUN)
                      ? (facingLeft ? -2 : 2) : 0;
    const int hatX  = static_cast<int>(position.x) - 4 + lean;
    const int hatY  = static_cast<int>(position.y) - 6;
    DrawRectangle(hatX,     hatY + 2, static_cast<int>(width) + 8, 4,  DARKBROWN);
    DrawRectangle(hatX + 6, hatY - 8, static_cast<int>(width) - 4, 10, DARKBROWN);

    // Squash / stretch hint: slightly taller when falling, wider when landing
    // (pure color/tint version — no rect resize to keep collider exact)

    // Gun
    DrawLineEx(chest, tip, GUN_THICKNESS, DARKGRAY);
    DrawCircleV(tip, 3.5f, GRAY);

    // Muzzle flash (tied to ATTACK interaction state)
    if (interactionState == PlayerInteractionState::ATTACK)
        DrawCircleV(tip, 6.0f, { 255, 220, 80, 200 });
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
