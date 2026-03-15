#pragma once
#include <raylib.h>
#include <array>
#include <frame.hpp> // for InputFrame
 
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
 
static_assert(static_cast<int>(Action::COUNT) == FRAME_ACTION_COUNT,
    "Action::COUNT and FRAME_ACTION_COUNT are out of sync — update frame.hpp");
 
struct ActionState {
    bool current  = false;
    bool previous = false;
};
 
class InputManager {
public:
    static constexpr int ACTION_COUNT = static_cast<int>(Action::COUNT);
 
    void LoadDefaultBindings();
    void Update();
    void Bind(Action action, int key);
 
    bool IsPressed (Action action) const; // rising edge  — was up, now down
    bool IsHeld    (Action action) const; // held down this frame
    bool IsReleased(Action action) const; // falling edge — was down, now up
 
    // Snapshot current state for archiving into FrameHistory
    InputFrame GetFrame() const;
 
private:
    std::array<ActionState, ACTION_COUNT> states;
    std::array<int,         ACTION_COUNT> keyBindings;
};
 
extern InputManager input;
 
