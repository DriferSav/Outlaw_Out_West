#pragma once
#include <raylib.h>
#include <cstdint>
#include <cstddef>
 
// Forward-declared here; full definition is in input.hpp.
// frame.hpp intentionally does NOT include input.hpp to keep the
// include order unambiguous: input.hpp -> frame.hpp -> player.hpp
enum class Action;
static constexpr int FRAME_ACTION_COUNT = 8; // must match Action::COUNT
 
// ---------------------------------------------------------------------------
// Per-frame snapshots — plain data, safe to memcpy
// ---------------------------------------------------------------------------
 
// Snapshot of every action's key-down state for one frame.
struct InputFrame {
    bool actions[FRAME_ACTION_COUNT] = {};
 
    // Convenience: is action A held in this snapshot?
    bool IsHeld(Action a) const {
        return actions[static_cast<int>(a)];
    }
};
 
// Snapshot of player physics + aim state for one frame.
struct PlayerFrame {
    Vector2 position = {};
    Vector2 velocity = {};
    float   aimAngle = 0.0f; // radians — 0 = right, positive = clockwise
    bool    onGround = false;
};
 
// One complete snapshot of everything needed to query the past.
struct FrameState {
    uint64_t    frameId   = 0;
    float       deltaTime = 0.0f;
    InputFrame  input;
    PlayerFrame player;
};
 
// ---------------------------------------------------------------------------
// FrameHistory — ring buffer of the last DEPTH frame snapshots
// ---------------------------------------------------------------------------
class FrameHistory {
public:
    static constexpr size_t DEPTH = 10; // ~166ms at 60 fps — covers coyote + input buffer
 
    // Archive a new snapshot. Silently overwrites the oldest entry when full.
    void Push(const FrameState& frame);
 
    // Look up a past frame.
    //   framesAgo = 0  ->  the most recently pushed frame
    //   framesAgo = 1  ->  one frame before that, etc.
    // Returns nullptr if framesAgo >= Count() (not enough history yet).
    const FrameState* Get(size_t framesAgo) const;
 
    // How many frames are stored right now (ramps 0 -> DEPTH on startup).
    size_t Count() const { return count; }
 
private:
    FrameState slots[DEPTH];
    size_t     head  = 0; // next write index
    size_t     count = 0; // valid entries
};
 
extern FrameHistory frameHistory;
 
