#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <player.hpp>
 
Player::Player() {
    position = { 100.0f, 300.0f };
    velocity = { 0.0f,   0.0f   };
}
 
PlayerFrame Player::GetFrame() const {
    return { position, velocity, onGround };
}
 
void Player::Update(float deltaTime) {
    HandleInput();
    ApplyGravity(deltaTime);
    Move(deltaTime);
    Collide();
}
 
void Player::Draw() {
    DrawRectangle(
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        static_cast<int>(width),
        static_cast<int>(height),
        RED);
}
 
void Player::HandleInput() {
    // --- Horizontal movement ---
    // IsHeld (not IsPressed) — movement should fire every frame the key is down
    float vx = 0.0f;
    if (input.IsHeld(Action::MOVE_RIGHT)) vx =  moveSpeed;
    if (input.IsHeld(Action::MOVE_LEFT))  vx = -moveSpeed;
    velocity.x = vx;
 
    // --- Jump: three paths, checked in priority order ---
    // Only one fires per frame — normal beats coyote beats buffered.
 
    // 1. Normal: grounded right now AND button just pressed this frame
    const bool normalJump = onGround && input.IsPressed(Action::JUMP);
 
    // 2. Coyote: airborne BUT walked off a ledge recently + button just pressed
    const bool coyoteJump = !onGround
                         && input.IsPressed(Action::JUMP)
                         && WasRecentlyGrounded();
 
    // 3. Buffered: landed this frame AND jump was pressed just before landing
    const bool bufferedJump = onGround && !normalJump && HasBufferedJump();
 
    if (normalJump || coyoteJump || bufferedJump) {
        velocity.y = -jumpForce;
        onGround   = false;
    }
}
 
void Player::ApplyGravity(float deltaTime) {
    if (!onGround) {
        // BUG FIX: accumulate into velocity, not position directly
        velocity.y += gravity * deltaTime;
        if (velocity.y > fallSpeed)
            velocity.y = fallSpeed;
    }
}
 
void Player::Move(float deltaTime) {
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;
}
 
void Player::Collide() {
    constexpr float groundY = 300.0f;
    if (position.y + height >= groundY) {
        position.y = groundY - height;
        velocity.y = 0.0f;
        onGround   = true;
    } else {
        onGround = false;
    }
}
 
// Scans back through FrameHistory checking the player.onGround field.
// Starts at 1 — we already know frame 0 (this frame) is NOT grounded.
bool Player::WasRecentlyGrounded() const {
    for (int i = 1; i <= COYOTE_FRAMES; i++) {
        const FrameState* f = frameHistory.Get(static_cast<size_t>(i));
        if (!f) break;                // not enough history yet
        if (f->player.onGround) return true;
    }
    return false;
}
 
// Scans back through FrameHistory looking for a JUMP rising edge
// (held this snapshot, not held the snapshot before it).
bool Player::HasBufferedJump() const {
    for (int i = 0; i < JUMP_BUFFER_FRAMES; i++) {
        const FrameState* cur  = frameHistory.Get(static_cast<size_t>(i));
        const FrameState* prev = frameHistory.Get(static_cast<size_t>(i + 1));
        if (!cur || !prev) break;
 
        const bool risingEdge = cur->input.IsHeld(Action::JUMP)
                             && !prev->input.IsHeld(Action::JUMP);
        if (risingEdge) return true;
    }
    return false;
}
 
Player player;
 
