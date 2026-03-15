#pragma once
#include <raylib.h>
#include <vector>

// Result of resolving a single AABB — tells the caller which faces were hit
struct ResolveResult {
    bool hitLeft   = false;
    bool hitRight  = false;
    bool hitTop    = false; // landed on top
    bool hitBottom = false; // bumped ceiling
};

// Standard four-sided AABB resolution.
ResolveResult ResolveAABB(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const Rectangle& solid);

// One-sided: only resolve the TOP face.
// Use when velocity.y > 0 (falling) AND player bottom was above platform last frame.
// Pass prevBottomY = position.y + h from the frame BEFORE Move() was called.
bool ResolveOneSided(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const Rectangle& platform,
    float prevBottomY);

// Resolve against every solid; returns OR of all wall contacts.
// onGround is set true if any top-face resolution occurred (never cleared here).
void ResolveAll(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const std::vector<Rectangle>& solids,
    bool& onGround,
    bool& touchingWallLeft,
    bool& touchingWallRight);

// Resolve one-sided platforms. fallThrough flag skips them entirely.
void ResolveOneSidedAll(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const std::vector<Rectangle>& platforms,
    float prevBottomY,
    bool  fallThrough,
    bool& onGround);
