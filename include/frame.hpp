#pragma once
#include <raylib.h>
#include <cstdint>
#include <cstddef>

enum class Action;
static constexpr int FRAME_ACTION_COUNT = 12; // must match Action::COUNT

struct InputFrame {
    bool actions[FRAME_ACTION_COUNT] = {};
    bool IsHeld(Action a) const {
        return actions[static_cast<int>(a)];
    }
};

struct PlayerFrame {
    Vector2 position = {};
    Vector2 velocity = {};
    float   aimAngle = 0.0f;
    bool    onGround = false;
};

struct FrameState {
    uint64_t    frameId   = 0;
    float       deltaTime = 0.0f;
    InputFrame  input;
    PlayerFrame player;
};

class FrameHistory {
public:
    static constexpr size_t DEPTH = 10;
    void Push(const FrameState& frame);
    const FrameState* Get(size_t framesAgo) const;
    size_t Count() const { return count; }
private:
    FrameState slots[DEPTH];
    size_t     head  = 0;
    size_t     count = 0;
};

extern FrameHistory frameHistory;
