#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <collision.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <enemy.hpp>
#include <player.hpp>
#include <cmath>
#include <algorithm>

Player player;

Player::Player() {
    position = { 100.0f, 300.0f };
    velocity = {};
    hp = MAX_HP;
}

PlayerFrame Player::GetFrame() const {
    return { position, velocity, aimAngle, onGround };
}

Vector2 Player::GunBarrelTip() const {
    Vector2 c = ChestCenter(position, width, height);
    return { c.x + cosf(aimAngle)*GUN_LENGTH, c.y + sinf(aimAngle)*GUN_LENGTH };
}

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------
void Player::TakeDamage(int amount) {
    if (invTimer > 0.0f || isDead) return;
    hp -= amount;
    invTimer = INV_DURATION;
    if (hp <= 0) { hp = 0; isDead = true; }
}

void Player::CollectItems(const std::vector<std::string>& ids) {
    for (const auto& id : ids) {
        Global::gameData.inventory.push_back(id);
        // Auto-try unlocking any gate that wants this key
        level.TryUnlockGate(id);
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------
void Player::Update(float dt) {
    if (isDead) return;
    prevBottomY = position.y + height;
    UpdateTimers(dt);
    UpdateAim();
    HandleInput();
    ApplyGravity(dt);
    Move(dt);
    Collide();
    CheckHazards();
    UpdateStates();
}

void Player::UpdateTimers(float dt) {
    if (invTimer     > 0.0f) invTimer     -= dt;
    if (shootCooldown> 0.0f) shootCooldown -= dt;
    if (reloadTimer  > 0.0f) {
        reloadTimer -= dt;
        if (reloadTimer <= 0.0f) { reloadTimer = 0.0f; ammo = MAX_AMMO; }
    }
    if (dashTimer    > 0.0f) dashTimer    -= dt;
    if (dashCooldown > 0.0f) dashCooldown -= dt;
    if (meleeTimer   > 0.0f) meleeTimer   -= dt;
    if (wallJumpLock > 0.0f) wallJumpLock -= dt;
}

// ---------------------------------------------------------------------------
// Aim  — stick locks angle, mouse only updates when mouse is active device
// ---------------------------------------------------------------------------
void Player::UpdateAim() {
    const Vector2 stick     = input.GetRightStick();
    const Vector2 mouseDelta = GetMouseDelta();
    const bool stickActive  = stick.x != 0.0f || stick.y != 0.0f;
    const bool mouseMoved   = fabsf(mouseDelta.x) > 0.5f || fabsf(mouseDelta.y) > 0.5f;

    if (stickActive) {
        aimAngle      = atan2f(stick.y, stick.x);
        lastAimDevice = AimDevice::GAMEPAD;
    } else if (mouseMoved) {
        lastAimDevice = AimDevice::MOUSE;
    }

    if (lastAimDevice == AimDevice::MOUSE) {
        Vector2 chest = ChestCenter(position, width, height);
        Vector2 mouse = GetMousePosition();
        aimAngle = atan2f(mouse.y - chest.y, mouse.x - chest.x);
    }

    facingLeft = cosf(aimAngle) < 0.0f;
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void Player::HandleInput() {
    // ---- Fall-through: hold DOWN while on a one-sided platform ----
    fallThrough = input.IsHeld(Action::MOVE_DOWN) && onGround;

    // ---- Horizontal movement (locked during wall jump) ----
    if (wallJumpLock <= 0.0f && dashTimer <= 0.0f) {
        float vx = 0.0f;
        if (input.IsHeld(Action::MOVE_RIGHT)) vx =  moveSpeed;
        if (input.IsHeld(Action::MOVE_LEFT))  vx = -moveSpeed;
        const Vector2 ls = input.GetLeftStick();
        if (fabsf(ls.x) * moveSpeed > fabsf(vx)) vx = ls.x * moveSpeed;
        velocity.x = vx;
    }

    // ---- Dash ----
    const bool canDash = dashTimer <= 0.0f && dashCooldown <= 0.0f;
    if (canDash && input.IsPressed(Action::DASH)) {
        // Dash in facing direction (or stick direction)
        const Vector2 ls = input.GetLeftStick();
        float dir = (fabsf(ls.x) > 0.1f) ? (ls.x > 0 ? 1.0f : -1.0f)
                                           : (facingLeft ? -1.0f : 1.0f);
        dashDirX      = dir;
        dashTimer     = DASH_DURATION;
        dashCooldown  = DASH_COOLDOWN;
        velocity.y    = 0.0f; // cancel vertical during dash
    }
    if (dashTimer > 0.0f) {
        velocity.x = dashDirX * DASH_SPEED;
        velocity.y = 0.0f;
    }

    // ---- Wall slide ----
    const bool onWall = !onGround && (
        (touchingWallLeft  && !facingLeft) ||
        (touchingWallRight &&  facingLeft));
    if (onWall && velocity.y > WALL_SLIDE_SPEED) {
        velocity.y = WALL_SLIDE_SPEED;
    }

    // ---- Jump cut ----
    if (jumpHeld && velocity.y < 0.0f && input.IsReleased(Action::JUMP)) {
        velocity.y *= JUMP_CUT_FACTOR;
        jumpHeld = false;
    }
    if (velocity.y >= 0.0f) jumpHeld = false;

    // ---- Jump launch ----
    const bool normalJump   = onGround && input.IsPressed(Action::JUMP);
    const bool coyoteJump   = !onGround && !onWall && input.IsPressed(Action::JUMP) && WasRecentlyGrounded();
    const bool bufferedJump = onGround && !normalJump && HasBufferedJump();
    const bool wallJump     = onWall   && input.IsPressed(Action::JUMP);

    if (normalJump || coyoteJump || bufferedJump) {
        velocity.y = -JUMP_FORCE;
        jumpHeld   = true;
        onGround   = false;
    } else if (wallJump) {
        float pushDir = touchingWallLeft ? 1.0f : -1.0f;
        velocity.x    = pushDir * WALL_JUMP_VX;
        velocity.y    = -WALL_JUMP_VY;
        jumpHeld      = true;
        wallJumpLock  = WALL_JUMP_LOCK_TIME;
        onGround      = false;
    }

    // ---- Reload ----
    if (input.IsPressed(Action::RELOAD) && ammo < MAX_AMMO && reloadTimer <= 0.0f)
        reloadTimer = RELOAD_TIME;

    // ---- Shoot ----
    const bool buttonShoot  = input.IsPressed(Action::SHOOT);
    const bool triggerShoot = input.GetRightTrigger() > 0.5f;
    if ((buttonShoot || triggerShoot) && shootCooldown <= 0.0f
        && ammo > 0 && reloadTimer <= 0.0f)
    {
        bulletPool.Spawn(GunBarrelTip(), aimAngle);
        shootCooldown = SHOOT_COOLDOWN;
        ammo--;
        if (ammo == 0) reloadTimer = RELOAD_TIME; // auto-reload on empty
    }

    // ---- Melee ----
    if (input.IsPressed(Action::MELEE) && meleeTimer <= 0.0f) {
        meleeTimer = MELEE_DURATION;
        // Sweep a hitbox in front of the player
        Vector2 tip = GunBarrelTip();
        enemyManager.HitAt(tip, MELEE_RANGE, 2);
    }

    // ---- Interact (pick up items, talk to NPCs) ----
    if (input.IsPressed(Action::INTERACT) && level.IsLoaded()) {
        Rectangle pb = { position.x, position.y, width, height };
        auto collected = level.CollectItems(pb);
        CollectItems(collected);
    }
}

// ---------------------------------------------------------------------------
// Physics
// ---------------------------------------------------------------------------
void Player::ApplyGravity(float dt) {
    if (onGround || dashTimer > 0.0f) return;
    float grav;
    if      (velocity.y < 0.0f && jumpHeld) grav = GRAVITY_RISING;
    else if (velocity.y > 0.0f)             grav = GRAVITY_FALLING;
    else                                    grav = GRAVITY_NORMAL;
    velocity.y += grav * dt;
    if (velocity.y > fallSpeed) velocity.y = fallSpeed;
}

void Player::Move(float dt) {
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
}

void Player::Collide() {
    onGround = touchingWallLeft = touchingWallRight = false;

    if (level.IsLoaded()) {
        ResolveAll(position, velocity, width, height,
                   level.GetSolidColliders(),
                   onGround, touchingWallLeft, touchingWallRight);
        ResolveOneSidedAll(position, velocity, width, height,
                           level.GetOneSidedColliders(),
                           prevBottomY, fallThrough, onGround);
    }

    // Screen edges
    if (position.x < 0.0f) { position.x = 0.0f; velocity.x = 0.0f; }
    const float rightEdge = (float)Global::SCREEN_WIDTH - width;
    if (position.x > rightEdge) { position.x = rightEdge; velocity.x = 0.0f; }

    // Death: fell off bottom
    if (position.y > (float)Global::SCREEN_HEIGHT + 120.0f) isDead = true;
}

void Player::CheckHazards() {
    if (!level.IsLoaded()) return;
    Rectangle pb = { position.x, position.y, width, height };
    int dmg = level.GetContactDamage(pb);
    if (dmg > 0) TakeDamage(dmg);
    dmg = enemyManager.GetContactDamage(pb);
    if (dmg > 0) TakeDamage(dmg);
}

// ---------------------------------------------------------------------------
// State derivation
// ---------------------------------------------------------------------------
void Player::UpdateStates() {
    if      (onGround)              environmentState = PlayerEnvironmentState::ON_GROUND;
    else if (velocity.y < 0.0f)    environmentState = PlayerEnvironmentState::RISING_IN_AIR;
    else                            environmentState = PlayerEnvironmentState::FALLING_IN_AIR;

    if (dashTimer > 0.0f)                                              movementState = PlayerMovementState::DASH;
    else if (!onGround && (touchingWallLeft || touchingWallRight))     movementState = PlayerMovementState::WALL_SLIDE;
    else if (environmentState != PlayerEnvironmentState::ON_GROUND)    movementState = PlayerMovementState::JUMP;
    else if (fabsf(velocity.x) > moveSpeed * 0.8f)                    movementState = PlayerMovementState::RUN;
    else if (fabsf(velocity.x) > 2.0f)                                movementState = PlayerMovementState::WALK;
    else                                                               movementState = PlayerMovementState::IDLE;

    if      (meleeTimer   > 0.0f)                          interactionState = PlayerInteractionState::MELEE;
    else if (shootCooldown> SHOOT_COOLDOWN * 0.6f)         interactionState = PlayerInteractionState::ATTACK;
    else                                                   interactionState = PlayerInteractionState::IDLE;
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void Player::Draw() const {
    const Vector2 chest = ChestCenter(position, width, height);
    const Vector2 tip   = GunBarrelTip();

    // Invincibility flicker
    if (invTimer > 0.0f && ((int)(invTimer * 12) % 2 == 0)) return;

    // Body color by state
    Color body;
    switch (movementState) {
        case PlayerMovementState::DASH:       body = {  80, 180, 255, 255 }; break; // cyan dash
        case PlayerMovementState::WALL_SLIDE: body = { 160,  80, 220, 255 }; break; // purple wall
        case PlayerMovementState::JUMP:       body = { 200,  60, 180, 255 }; break;
        case PlayerMovementState::RUN:        body = { 240,  70,  50, 255 }; break;
        case PlayerMovementState::WALK:       body = { 210,  55,  55, 255 }; break;
        default:                              body = { 180,  40,  40, 255 }; break;
    }
    if (interactionState == PlayerInteractionState::ATTACK) body = { 255, 220, 180, 255 };
    if (interactionState == PlayerInteractionState::MELEE)  body = { 255, 255, 100, 255 };

    DrawRectangle((int)position.x, (int)position.y, (int)width, (int)height, body);

    // Hat
    const int lean = (movementState == PlayerMovementState::RUN) ? (facingLeft ? -2 : 2) : 0;
    const int hx   = (int)position.x - 4 + lean;
    const int hy   = (int)position.y - 6;
    DrawRectangle(hx,     hy+2, (int)width+8, 4,  DARKBROWN);
    DrawRectangle(hx+6,   hy-8, (int)width-4, 10, DARKBROWN);

    // Melee arc
    if (meleeTimer > 0.0f) {
        float pct = meleeTimer / MELEE_DURATION;
        DrawCircleLines((int)chest.x, (int)chest.y, MELEE_RANGE * (1.0f - pct + 0.4f), {255,255,100,160});
    }

    // Gun
    DrawLineEx(chest, tip, GUN_THICKNESS, DARKGRAY);
    DrawCircleV(tip, 3.5f, GRAY);
    if (interactionState == PlayerInteractionState::ATTACK)
        DrawCircleV(tip, 6.0f, {255,220,80,200});
}

// ---------------------------------------------------------------------------
// Frame history queries
// ---------------------------------------------------------------------------
bool Player::WasRecentlyGrounded() const {
    for (int i = 1; i <= COYOTE_FRAMES; i++) {
        const FrameState* f = frameHistory.Get((size_t)i);
        if (!f) break;
        if (f->player.onGround) return true;
    }
    return false;
}

bool Player::HasBufferedJump() const {
    for (int i = 0; i < JUMP_BUFFER_FRAMES; i++) {
        const FrameState* cur  = frameHistory.Get((size_t)i);
        const FrameState* prev = frameHistory.Get((size_t)(i+1));
        if (!cur || !prev) break;
        if (cur->input.IsHeld(Action::JUMP) && !prev->input.IsHeld(Action::JUMP)) return true;
    }
    return false;
}
