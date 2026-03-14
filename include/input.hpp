#pragma once
#include <raylib.h>
#include <array>

enum class Action {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,
    PAUSE,
    ENTER,
    COUNT
};

struct ActionState {
    bool current = false;
    bool previous = false;
};

class InputManager {
    public:
        static constexpr int ACTION_COUNT = static_cast<int>(Action::COUNT);

        void LoadDefaultBindings();

        void Update();
        void Bind(Action action, int key);

        bool IsPressed(Action action) const;
        bool IsHeld(Action action) const;
        bool IsReleased(Action action) const;

    private:
        std::array<ActionState, ACTION_COUNT> states;
        std::array<int, ACTION_COUNT> keyBindings;
};

extern InputManager input;
