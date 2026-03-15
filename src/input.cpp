#include <input.hpp>
#include <cmath>
 
// ---------------------------------------------------------------------------
// Default bindings
//
// Keyboard/mouse          Gamepad (Xbox / PS layout)
// ─────────────────────   ───────────────────────────────────────────────
// A / D                   Left stick X  (analog — handled in Player)
// W / S                   Left stick Y  (analog — handled in Player)
// A / D also              D-pad left / right  (digital fallback)
// Space                   Cross / A  (GAMEPAD_BUTTON_RIGHT_FACE_DOWN)
// Mouse left click        Right bumper  (GAMEPAD_BUTTON_RIGHT_TRIGGER_1)
//                         Right trigger axis > 0.5 handled in Player directly
// Escape                  Start  (GAMEPAD_BUTTON_MIDDLE_RIGHT)
// Enter                   Cross / A  (same as Jump — fine for menus)
// ---------------------------------------------------------------------------
void InputManager::LoadDefaultBindings() {
    // Keyboard
    Bind(Action::MOVE_LEFT,  KEY_A);
    Bind(Action::MOVE_RIGHT, KEY_D);
    Bind(Action::MOVE_UP,    KEY_W);
    Bind(Action::MOVE_DOWN,  KEY_S);
    Bind(Action::JUMP,       KEY_SPACE);
    Bind(Action::SHOOT,      BIND_MOUSE_LEFT);
    Bind(Action::PAUSE,      KEY_ESCAPE);
    Bind(Action::ENTER,      KEY_ENTER);
 
    // Gamepad — D-pad for movement, face buttons for actions
    BindGamepad(Action::MOVE_LEFT,  GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    BindGamepad(Action::MOVE_RIGHT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    BindGamepad(Action::MOVE_UP,    GAMEPAD_BUTTON_LEFT_FACE_UP);
    BindGamepad(Action::MOVE_DOWN,  GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    BindGamepad(Action::JUMP,       GAMEPAD_BUTTON_RIGHT_FACE_DOWN);   // Cross / A
    BindGamepad(Action::SHOOT,      GAMEPAD_BUTTON_RIGHT_TRIGGER_1);   // RB
    BindGamepad(Action::PAUSE,      GAMEPAD_BUTTON_MIDDLE_RIGHT);       // Start
    BindGamepad(Action::ENTER,      GAMEPAD_BUTTON_RIGHT_FACE_DOWN);   // Cross / A
}
 
void InputManager::Bind(Action action, int key) {
    keyBindings[static_cast<int>(action)] = key;
}
 
void InputManager::BindGamepad(Action action, int gamepadButton) {
    padBindings[static_cast<int>(action)] = gamepadButton;
}
 
void InputManager::Update() {
    const bool padAvail = IsGamepadAvailable(0);
 
    // --- Digital action states ---
    // Each action is true if EITHER the keyboard/mouse binding OR the
    // gamepad button binding is currently down.
    for (int i = 0; i < ACTION_COUNT; i++) {
        ActionState& s = states[i];
        s.previous = s.current;
 
        // Keyboard / mouse
        bool keyState = false;
        const int key = keyBindings[i];
        if (key < 0) {
            keyState = IsMouseButtonDown(-(key + 1));
        } else if (key > 0) {
            keyState = IsKeyDown(key);
        }
 
        // Gamepad button
        bool padState = false;
        const int btn = padBindings[i];
        if (btn > 0 && padAvail) {
            padState = IsGamepadButtonDown(0, btn);
        }
 
        s.current = keyState || padState;
    }
 
    // --- Analog axes (zeroed when below deadzone or no gamepad) ---
    leftStick  = {};
    rightStick = {};
    leftTrig   = 0.0f;
    rightTrig  = 0.0f;
 
    if (padAvail) {
        // Left stick
        const float lx  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        const float ly  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (sqrtf(lx * lx + ly * ly) > STICK_DEADZONE)
            leftStick = { lx, ly };
 
        // Right stick
        const float rx  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        const float ry  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        if (sqrtf(rx * rx + ry * ry) > STICK_DEADZONE)
            rightStick = { rx, ry };
 
        // Triggers — remap from [-1, 1] to [0, 1]
        // Raylib reports triggers as -1 at rest, +1 fully pressed
        leftTrig  = (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER)  + 1.0f) * 0.5f;
        rightTrig = (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) + 1.0f) * 0.5f;
    }
}
 
// ---------------------------------------------------------------------------
// Digital queries
// ---------------------------------------------------------------------------
bool InputManager::IsPressed(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return s.current && !s.previous;
}
 
bool InputManager::IsHeld(Action action) const {
    return states[static_cast<int>(action)].current;
}
 
bool InputManager::IsReleased(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return !s.current && s.previous;
}
 
// ---------------------------------------------------------------------------
// Analog queries
// ---------------------------------------------------------------------------
Vector2 InputManager::GetLeftStick()  const { return leftStick;  }
Vector2 InputManager::GetRightStick() const { return rightStick; }
float   InputManager::GetLeftTrigger()  const { return leftTrig;  }
float   InputManager::GetRightTrigger() const { return rightTrig; }
bool    InputManager::IsGamepadConnected() const { return IsGamepadAvailable(0); }
 
// ---------------------------------------------------------------------------
// Frame snapshot
// ---------------------------------------------------------------------------
InputFrame InputManager::GetFrame() const {
    InputFrame f;
    for (int i = 0; i < ACTION_COUNT; i++)
        f.actions[i] = states[i].current;
    return f;
}
 
InputManager input;
 
