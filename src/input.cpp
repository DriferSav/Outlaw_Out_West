#include <input.hpp>
#include <cmath>

void InputManager::LoadDefaultBindings() {
    Bind(Action::MOVE_LEFT,  KEY_A);
    Bind(Action::MOVE_RIGHT, KEY_D);
    Bind(Action::MOVE_UP,    KEY_W);
    Bind(Action::MOVE_DOWN,  KEY_S);
    Bind(Action::JUMP,       KEY_SPACE);
    Bind(Action::SHOOT,      BIND_MOUSE_LEFT);
    Bind(Action::PAUSE,      KEY_ESCAPE);
    Bind(Action::ENTER,      KEY_ENTER);
    Bind(Action::DASH,       KEY_LEFT_SHIFT);
    Bind(Action::MELEE,      KEY_E);
    Bind(Action::RELOAD,     KEY_R);
    Bind(Action::INTERACT,   KEY_F);

    BindGamepad(Action::MOVE_LEFT,  GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    BindGamepad(Action::MOVE_RIGHT, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
    BindGamepad(Action::MOVE_UP,    GAMEPAD_BUTTON_LEFT_FACE_UP);
    BindGamepad(Action::MOVE_DOWN,  GAMEPAD_BUTTON_LEFT_FACE_DOWN);
    BindGamepad(Action::JUMP,       GAMEPAD_BUTTON_RIGHT_FACE_DOWN);  // A / Cross
    BindGamepad(Action::SHOOT,      GAMEPAD_BUTTON_RIGHT_TRIGGER_1);  // RB
    BindGamepad(Action::PAUSE,      GAMEPAD_BUTTON_MIDDLE_RIGHT);      // Start
    BindGamepad(Action::ENTER,      GAMEPAD_BUTTON_RIGHT_FACE_DOWN);  // A / Cross
    BindGamepad(Action::DASH,       GAMEPAD_BUTTON_LEFT_TRIGGER_1);   // LB
    BindGamepad(Action::MELEE,      GAMEPAD_BUTTON_RIGHT_FACE_UP);    // Y / Triangle
    BindGamepad(Action::RELOAD,     GAMEPAD_BUTTON_RIGHT_FACE_LEFT);  // X / Square
    BindGamepad(Action::INTERACT,   GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); // B / Circle
}

void InputManager::Bind(Action a, int key)         { keyBindings[static_cast<int>(a)] = key; }
void InputManager::BindGamepad(Action a, int btn)  { padBindings[static_cast<int>(a)] = btn; }

void InputManager::Update() {
    const bool pad = IsGamepadAvailable(0);
    for (int i = 0; i < ACTION_COUNT; i++) {
        ActionState& s = states[i];
        s.previous = s.current;
        bool ks = false, ps = false;
        const int key = keyBindings[i];
        if      (key < 0)  ks = IsMouseButtonDown(-(key + 1));
        else if (key > 0)  ks = IsKeyDown(key);
        const int btn = padBindings[i];
        if (btn > 0 && pad) ps = IsGamepadButtonDown(0, btn);
        s.current = ks || ps;
    }
    leftStick = rightStick = {};
    leftTrig = rightTrig = 0.0f;
    if (pad) {
        const float lx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        const float ly = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
        if (sqrtf(lx*lx + ly*ly) > STICK_DEADZONE) leftStick = {lx, ly};
        const float rx = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
        const float ry = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);
        if (sqrtf(rx*rx + ry*ry) > STICK_DEADZONE) rightStick = {rx, ry};
        leftTrig  = (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_TRIGGER)  + 1.0f) * 0.5f;
        rightTrig = (GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_TRIGGER) + 1.0f) * 0.5f;
    }
}

bool    InputManager::IsPressed (Action a) const { const auto& s=states[static_cast<int>(a)]; return  s.current && !s.previous; }
bool    InputManager::IsHeld    (Action a) const { return states[static_cast<int>(a)].current; }
bool    InputManager::IsReleased(Action a) const { const auto& s=states[static_cast<int>(a)]; return !s.current &&  s.previous; }
Vector2 InputManager::GetLeftStick()    const { return leftStick;  }
Vector2 InputManager::GetRightStick()   const { return rightStick; }
float   InputManager::GetLeftTrigger()  const { return leftTrig;   }
float   InputManager::GetRightTrigger() const { return rightTrig;  }
bool    InputManager::IsGamepadConnected() const { return IsGamepadAvailable(0); }

InputFrame InputManager::GetFrame() const {
    InputFrame f;
    for (int i = 0; i < ACTION_COUNT; i++) f.actions[i] = states[i].current;
    return f;
}

InputManager input;
