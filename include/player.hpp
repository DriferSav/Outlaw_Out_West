#pragma once
#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <string>
#include <vector>

enum class PlayerMovementState    { IDLE, WALK, RUN, JUMP, DASH, WALL_SLIDE };
enum class PlayerEnvironmentState { ON_GROUND, RISING_IN_AIR, FALLING_IN_AIR };
enum class PlayerInteractionState { IDLE, ATTACK, MELEE };

class Player {
public:
    Player();

    void Update(float dt);
    void Draw()  const;

    PlayerFrame GetFrame() const;

    // Geometry
    Vector2 GunBarrelTip() const;
    float   GetAimAngle()  const { return aimAngle; }

    // State
    bool IsDead()  const { return isDead;  }
    int  GetHp()   const { return hp;      }
    int  GetMaxHp()const { return MAX_HP;  }
    int  GetAmmo() const { return ammo;    }
    int  GetMaxAmmo() const { return MAX_AMMO; }
    bool IsReloading()  const { return reloadTimer > 0.0f; }
    float GetReloadProgress()   const { return 1.0f - (reloadTimer / RELOAD_TIME); }
    float GetDashCooldownPct()  const { return dashCooldown <= 0.0f ? 1.0f : 1.0f - dashCooldown / DASH_COOLDOWN; }

    PlayerMovementState    GetMovementState()    const { return movementState;    }
    PlayerEnvironmentState GetEnvironmentState() const { return environmentState; }
    PlayerInteractionState GetInteractionState() const { return interactionState; }

    // Apply damage from outside (enemies, hazards)
    void TakeDamage(int amount);

    // Collect inventory items
    void CollectItems(const std::vector<std::string>& ids);

private:
    // ---- Transform ----
    Vector2 position = {};
    Vector2 velocity = {};
    float   prevBottomY = 0.0f; // bottom Y before Move() — needed for one-sided platforms
    float   width  = 32.0f;
    float   height = 48.0f;

    // ---- Derived state ----
    PlayerMovementState    movementState    = PlayerMovementState::IDLE;
    PlayerEnvironmentState environmentState = PlayerEnvironmentState::ON_GROUND;
    PlayerInteractionState interactionState = PlayerInteractionState::IDLE;

    // ---- Physics ----
    float moveSpeed = 220.0f;
    float fallSpeed = 900.0f;
    bool  onGround        = false;
    bool  touchingWallLeft  = false;
    bool  touchingWallRight = false;
    bool  isDead = false;

    // ---- Health ----
    static constexpr int   MAX_HP       = 5;
    static constexpr float INV_DURATION = 0.8f;
    int   hp        = MAX_HP;
    float invTimer  = 0.0f;   // invincibility after hit

    // ---- Jump ----
    bool jumpHeld = false;
    static constexpr float JUMP_FORCE      = 480.0f;
    static constexpr float JUMP_CUT_FACTOR = 0.45f;
    static constexpr float GRAVITY_RISING  = 700.0f;
    static constexpr float GRAVITY_NORMAL  = 1200.0f;
    static constexpr float GRAVITY_FALLING = 1500.0f;
    static constexpr int   COYOTE_FRAMES   = 8;
    static constexpr int   JUMP_BUFFER_FRAMES = 10;

    // ---- Wall jump ----
    static constexpr float WALL_JUMP_VX       = 260.0f;
    static constexpr float WALL_JUMP_VY       = 420.0f;
    static constexpr float WALL_SLIDE_SPEED   = 80.0f;   // slow fall while on wall
    static constexpr float WALL_JUMP_LOCK_TIME= 0.18f;   // horizontal control lockout
    float wallJumpLock = 0.0f;  // > 0 means horizontal input is locked

    // ---- Dash ----
    static constexpr float DASH_SPEED    = 520.0f;
    static constexpr float DASH_DURATION = 0.14f;
    static constexpr float DASH_COOLDOWN = 0.8f;
    float dashTimer    = 0.0f; // > 0 = currently dashing
    float dashCooldown = 0.0f;
    float dashDirX     = 1.0f; // direction of active dash

    // ---- Ammo / reload ----
    static constexpr int   MAX_AMMO    = 6;
    static constexpr float RELOAD_TIME = 2.0f;
    int   ammo        = MAX_AMMO;
    float reloadTimer = 0.0f;
    float shootCooldown = 0.0f;
    static constexpr float SHOOT_COOLDOWN = 0.15f;

    // ---- Melee ----
    static constexpr float MELEE_DURATION = 0.22f;
    static constexpr float MELEE_RANGE    = 44.0f;
    float meleeTimer = 0.0f;

    // ---- Aim ----
    enum class AimDevice { MOUSE, GAMEPAD };
    float     aimAngle      = 0.0f;
    bool      facingLeft    = false;
    AimDevice lastAimDevice = AimDevice::MOUSE;
    static constexpr float GUN_LENGTH    = 28.0f;
    static constexpr float GUN_THICKNESS = 5.0f;

    // ---- Fall-through flag ----
    bool fallThrough = false; // true while player holds DOWN on a one-sided platform

    // ---- Private methods ----
    void UpdateTimers(float dt);
    void UpdateAim();
    void HandleInput();
    void ApplyGravity(float dt);
    void Move(float dt);
    void Collide();
    void CheckHazards();
    void UpdateStates();

    bool WasRecentlyGrounded() const;
    bool HasBufferedJump()     const;

    // Helpers
    static Vector2 ChestCenter(Vector2 pos, float w, float h) {
        return { pos.x + w * 0.5f, pos.y + h * 0.5f - 4.0f };
    }
};

extern Player player;
