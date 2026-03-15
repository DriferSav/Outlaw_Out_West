#include <frame.hpp>
 
// ---------------------------------------------------------------------------
// FrameHistory
// ---------------------------------------------------------------------------
 
void FrameHistory::Push(const FrameState& frame) {
    slots[head] = frame;
    head        = (head + 1) % DEPTH;
    if (count < DEPTH) count++;
}
 
const FrameState* FrameHistory::Get(size_t framesAgo) const {
    if (framesAgo >= count) return nullptr;
    // head-1 = most recent write; walk back framesAgo steps
    size_t idx = (head + DEPTH - 1 - framesAgo) % DEPTH;
    return &slots[idx];
}
 
FrameHistory frameHistory;
 
