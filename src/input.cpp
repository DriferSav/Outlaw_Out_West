#include <input.hpp>

void InputManager::LoadDefaultBindings() {
    Bind(Action::MOVE_LEFT, KEY_A);
    Bind(Action::MOVE_RIGHT, KEY_D);
    Bind(Action::MOVE_UP, KEY_W);
    Bind(Action::MOVE_DOWN, KEY_S);
    Bind(Action::JUMP, KEY_SPACE);
    Bind(Action::PAUSE, KEY_ESCAPE);
    Bind(Action::ENTER, KEY_ENTER);
}

void InputManager::Bind(Action action, int key) {
    keyBindings[static_cast<int>(action)] = key;
}

void InputManager::Update() {
    for (int i = 0; i < ACTION_COUNT; i++) {
        ActionState& state = states[i];
        state.previous = state.current;
        int key = keyBindings[i];
        state.current = IsKeyDown(key); // Raylib function
    }
}

bool InputManager::IsHeld(Action action) const {
    return states[static_cast<int>(action)].current;
}

bool InputManager::IsPressed(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return s.current && !s.previous;
}

bool InputManager::IsReleased(Action action) const {
    const ActionState& s = states[static_cast<int>(action)];
    return !s.current && s.previous;
}

InputManager input;
