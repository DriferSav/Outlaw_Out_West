#include <input.hpp>
 
void InputManager::LoadDefaultBindings() {
    Bind(Action::MOVE_LEFT,  KEY_A);
    Bind(Action::MOVE_RIGHT, KEY_D);
    Bind(Action::MOVE_UP,    KEY_W);
    Bind(Action::MOVE_DOWN,  KEY_S);
    Bind(Action::JUMP,       KEY_SPACE);
    Bind(Action::PAUSE,      KEY_ESCAPE);
    Bind(Action::ENTER,      KEY_ENTER);
}
 
void InputManager::Bind(Action action, int key) {
    keyBindings[static_cast<int>(action)] = key;
}
 
void InputManager::Update() {
    for (int i = 0; i < ACTION_COUNT; i++) {
        ActionState& s = states[i];
        s.previous = s.current;
        s.current  = IsKeyDown(keyBindings[i]);
    }
}
 
// Rising edge: was up last frame, down this frame
bool InputManager::IsPressed(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return s.current && !s.previous;
}
 
// Held down this frame (regardless of last frame)
bool InputManager::IsHeld(Action action) const {
    return states[static_cast<int>(action)].current;
}
 
// Falling edge: was down last frame, up this frame
bool InputManager::IsReleased(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return !s.current && s.previous;
}
 
InputFrame InputManager::GetFrame() const {
    InputFrame f;
    for (int i = 0; i < ACTION_COUNT; i++) {
        f.actions[i] = states[i].current;
    }
    return f;
}
 
InputManager input;
 
