#include <input.hpp>
#include <cmath>
 
void InputManager::LoadDefaultBindings() {
    Bind(Action::MOVE_LEFT,  KEY_A);
    Bind(Action::MOVE_RIGHT, KEY_D);
    Bind(Action::MOVE_UP,    KEY_W);
    Bind(Action::MOVE_DOWN,  KEY_S);
    Bind(Action::JUMP,       KEY_SPACE);
    Bind(Action::SHOOT,      BIND_MOUSE_LEFT); // left click; rebind to KEY_Z for gamepad-only
    Bind(Action::PAUSE,      KEY_ESCAPE);
    Bind(Action::ENTER,      KEY_ENTER);
}
 
void InputManager::Bind(Action action, int key) {
    keyBindings[static_cast<int>(action)] = key;
}
 
void InputManager::Update() {
    // --- Action states ---
    for (int i = 0; i < ACTION_COUNT; i++) {
        ActionState& s = states[i];
        s.previous = s.current;
 
        const int key = keyBindings[i];
        if (key == 0) {
            s.current = false;                         // unbound slot
        } else if (key < 0) {
            // Negative sentinel → mouse button
            // -1 → button 0 (left), -2 → button 1 (right), etc.
            s.current = IsMouseButtonDown(-(key + 1));
        } else {
            s.current = IsKeyDown(key);
        }
    }
 
    // --- Right joystick (gamepad 0) ---
    // Also check right trigger (axis 5) as an alternative shoot input — handled in Player.
    rightStick = {};
    if (IsGamepadAvailable(0)) {
        const float rx  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        const float ry  = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        const float mag = sqrtf(rx * rx + ry * ry);
        if (mag > STICK_DEADZONE) {
            rightStick = { rx, ry };
        }
    }
}
 
bool InputManager::IsPressed(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return s.current && !s.previous;   // rising edge
}
 
bool InputManager::IsHeld(Action action) const {
    return states[static_cast<int>(action)].current;
}
 
bool InputManager::IsReleased(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return !s.current && s.previous;   // falling edge
}
 
Vector2 InputManager::GetRightStick() const {
    return rightStick; // (0,0) when below deadzone or no gamepad
}
 
InputFrame InputManager::GetFrame() const {
    InputFrame f;
    for (int i = 0; i < ACTION_COUNT; i++) {
        f.actions[i] = states[i].current;
    }
    return f;
}
 
InputManager input;
 
