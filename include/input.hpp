#pragma once
#include <raylib.h>
#include <array>
#include <frame.hpp>

constexpr int BIND_MOUSE_LEFT   = -1;
constexpr int BIND_MOUSE_RIGHT  = -2;
constexpr int BIND_MOUSE_MIDDLE = -3;

enum class Action {
    MOVE_LEFT,   // 0
    MOVE_RIGHT,  // 1
    MOVE_UP,     // 2
    MOVE_DOWN,   // 3
    JUMP,        // 4
    SHOOT,       // 5
    PAUSE,       // 6
    ENTER,       // 7
    DASH,        // 8  — Shift / LB
    MELEE,       // 9  — E / Y (Triangle)
    RELOAD,      // 10 — R / X (Square)
    INTERACT,    // 11 — F / B (Circle) — pick up keys, open gates
    COUNT        // 12
};

static_assert(static_cast<int>(Action::COUNT) == FRAME_ACTION_COUNT,
    "Action::COUNT and FRAME_ACTION_COUNT are out of sync — update frame.hpp");

struct ActionState {
    bool current  = false;
    bool previous = false;
};

class InputManager {
public:
    static constexpr int   ACTION_COUNT   = static_cast<int>(Action::COUNT);
    static constexpr float STICK_DEADZONE = 0.20f;

    void LoadDefaultBindings();
    void Update();
    void Bind(Action action, int key);
    void BindGamepad(Action action, int gamepadButton);

    bool IsPressed (Action action) const;
    bool IsHeld    (Action action) const;
    bool IsReleased(Action action) const;

    Vector2 GetLeftStick()    const;
    Vector2 GetRightStick()   const;
    float   GetLeftTrigger()  const;
    float   GetRightTrigger() const;
    bool    IsGamepadConnected() const;

    InputFrame GetFrame() const;

private:
    std::array<ActionState, ACTION_COUNT> states      = {};
    std::array<int,         ACTION_COUNT> keyBindings = {};
    std::array<int,         ACTION_COUNT> padBindings = {};
    Vector2 leftStick  = {};
    Vector2 rightStick = {};
    float   leftTrig   = 0.0f;
    float   rightTrig  = 0.0f;
};

extern InputManager input;
