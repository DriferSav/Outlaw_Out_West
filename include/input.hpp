#pragma once
#include <raylib.h>
#include <array>
#include <frame.hpp>
 
// ---------------------------------------------------------------------------
// Special key binding constants
//
// Raylib's IsKeyDown() only handles keyboard keys. To bind mouse buttons
// through the same system, we use negative sentinel values. InputManager::Update()
// checks the sign and routes to IsMouseButtonDown() instead.
//
//   keyBinding > 0  → IsKeyDown(keyBinding)
//   keyBinding < 0  → IsMouseButtonDown(-(keyBinding + 1))
//     -1 → MOUSE_BUTTON_LEFT  (0)
//     -2 → MOUSE_BUTTON_RIGHT (1)
//     -3 → MOUSE_BUTTON_MIDDLE(2)
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
    SHOOT,   // left click / left trigger
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
    void Bind(Action action, int key);
 
    bool IsPressed (Action action) const;
    bool IsHeld    (Action action) const;
    bool IsReleased(Action action) const;
 
    // Right joystick — returns (0,0) when below deadzone or no gamepad connected.
    Vector2 GetRightStick() const;
 
    InputFrame GetFrame() const;
 
private:
    std::array<ActionState, ACTION_COUNT> states      = {};
    std::array<int,         ACTION_COUNT> keyBindings = {};
    Vector2 rightStick = {};
};
 
extern InputManager input;
 
