#pragma once
#include <raylib.h>
#include <array>
#include <frame.hpp>
 
// ---------------------------------------------------------------------------
// Binding sentinels
//
// keyBindings slot:
//   0              → unbound
//   > 0            → IsKeyDown(key)
//  -1              → IsMouseButtonDown(MOUSE_BUTTON_LEFT)
//  -2              → IsMouseButtonDown(MOUSE_BUTTON_RIGHT)
//  -3              → IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)
//
// padBindings slot:
//   0              → unbound
//   > 0            → IsGamepadButtonDown(0, btn)
//                    Use Raylib's GamepadButton enum values directly.
// ---------------------------------------------------------------------------
constexpr int BIND_MOUSE_LEFT   = -1;
constexpr int BIND_MOUSE_RIGHT  = -2;
constexpr int BIND_MOUSE_MIDDLE = -3;
 
enum class Action {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,
    SHOOT,
    PAUSE,
    ENTER,
    COUNT
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
 
    // Bind a keyboard key or mouse sentinel to an action.
    void Bind(Action action, int key);
 
    // Bind a gamepad button (Raylib GamepadButton enum value) to an action.
    // This binding is OR'd with the keyboard binding — either triggers the action.
    void BindGamepad(Action action, int gamepadButton);
 
    // --- Digital action queries ---
    bool IsPressed (Action action) const; // rising edge  — was up, now down
    bool IsHeld    (Action action) const; // held this frame (any duration)
    bool IsReleased(Action action) const; // falling edge — was down, now up
 
    // --- Analog stick queries (zeroed when below deadzone or no gamepad) ---
    Vector2 GetLeftStick()  const;
    Vector2 GetRightStick() const;
 
    // --- Trigger queries (0.0–1.0, zeroed when no gamepad) ---
    float GetLeftTrigger()  const;
    float GetRightTrigger() const;
 
    // --- Utility ---
    bool IsGamepadConnected() const;
 
    // Snapshot all digital action states for FrameHistory.
    InputFrame GetFrame() const;
 
private:
    std::array<ActionState, ACTION_COUNT> states      = {};
    std::array<int,         ACTION_COUNT> keyBindings = {};
    std::array<int,         ACTION_COUNT> padBindings = {};
 
    // Cached each frame by Update()
    Vector2 leftStick  = {};
    Vector2 rightStick = {};
    float   leftTrig   = 0.0f;
    float   rightTrig  = 0.0f;
};
 
extern InputManager input;
 
