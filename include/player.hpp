#pragma once
#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>

// ---------------------------------------------------------------------------
// State enums — derived each frame from physics, not set by input directly
// ---------------------------------------------------------------------------
enum class PlayerMovementState {
    IDLE,   // on ground, no horizontal velocity
    WALK,   // on ground, moving at less than ~80% of moveSpeed
    RUN,    // on ground, moving at ~80%+ of moveSpeed
    JUMP    // in the air (rising or falling)
};

enum class PlayerEnvironmentState {
    ON_GROUND,
    RISING_IN_AIR,
    FALLING_IN_AIR
};

enum class PlayerInteractionState {
    IDLE,    // no action
    ATTACK   // shot fired within the muzzle-flash window
};

// ---------------------------------------------------------------------------
// Player
// ---------------------------------------------------------------------------
class Player {
public:
    Player();

    void Update(float deltaTime);
    void Draw() const;

    PlayerFrame GetFrame() const;

    // Public accessors
    Vector2 GunBarrelTip() const;
    float   GetAimAngle()  const { return aimAngle; }
    bool    IsDead()       const { return isDead;   }

    PlayerMovementState    GetMovementState()    const { return movementState;    }
    PlayerEnvironmentState GetEnvironmentState() const { return environmentState; }
    PlayerInteractionState GetInteractionState() const { return interactionState; }

private:
    // ---- Transform ----
    Vector2 position = {};
    Vector2 velocity = {};
    float   width    = 32.0f;
    float   height   = 48.0f;

    // ---- Movement ----
    float moveSpeed = 220.0f;
    float fallSpeed = 900.0f;
    bool  onGround  = false;
    bool  isDead    = false;

    // ---- Variable jump ----
    // jumpHeld tracks whether the player is still holding jump after launch.
    // While true and rising, reduced gravity gives a taller arc.
    // Early release multiplies vy by JUMP_CUT_FACTOR for a short hop.
    bool jumpHeld = false;

    static constexpr float JUMP_FORCE      = 480.0f;
    static constexpr float JUMP_CUT_FACTOR = 0.45f;
    static constexpr float GRAVITY_RISING  = 700.0f;  // while rising + held
    static constexpr float GRAVITY_NORMAL  = 1200.0f; // apex / cut arc
    static constexpr float GRAVITY_FALLING = 1500.0f; // snappy descent

    static constexpr int   COYOTE_FRAMES      = 8;
    static constexpr int   JUMP_BUFFER_FRAMES = 10;

    // ---- Aim ----
    // lastAimDevice ensures the gun stays frozen at its last gamepad angle
    // when the stick returns to center, instead of snapping to the mouse.
    enum class AimDevice { MOUSE, GAMEPAD };

    float     aimAngle      = 0.0f;
    bool      facingLeft    = false;
    AimDevice lastAimDevice = AimDevice::MOUSE;

    // ---- Shoot ----
    float shootCooldown = 0.0f;
    static constexpr float SHOOT_COOLDOWN = 0.25f;
    static constexpr float GUN_LENGTH     = 28.0f;
    static constexpr float GUN_THICKNESS  = 5.0f;

    // ---- Derived state (updated end of every tick) ----
    PlayerMovementState    movementState    = PlayerMovementState::IDLE;
    PlayerEnvironmentState environmentState = PlayerEnvironmentState::ON_GROUND;
    PlayerInteractionState interactionState = PlayerInteractionState::IDLE;

    // ---- Private methods ----
    void UpdateAim();
    void HandleInput();
    void ApplyGravity(float deltaTime);
    void Move(float deltaTime);
    void Collide();
    void UpdateStates();   // derives all three state enums from physics

    bool WasRecentlyGrounded() const;
    bool HasBufferedJump()     const;
};

extern Player player;
