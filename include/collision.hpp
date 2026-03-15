#pragma once
#include <raylib.h>
#include <vector>
 
// ---------------------------------------------------------------------------
// AABB collision resolution
//
// How it works:
//   After the player moves, we check if their rect overlaps any solid rect.
//   If it does, we measure penetration depth on X and Y separately, then
//   push the player out on whichever axis has LESS overlap — because that
//   axis is almost certainly the one they came from this frame.
//
//   Resolving Y last (by calling this after MoveX then MoveY separately, or
//   by the min-axis logic below) ensures landing/ceiling hits are accurate.
//
// Parameters:
//   position  — top-left of the moving entity (modified in-place)
//   velocity  — entity velocity (zeroed on the resolved axis)
//   w, h      — entity dimensions
//   solid     — the static rectangle to resolve against
//   onGround  — set to true if entity lands on top of solid (never cleared here)
// ---------------------------------------------------------------------------
void ResolveAABB(
    Vector2&         position,
    Vector2&         velocity,
    float            w,
    float            h,
    const Rectangle& solid,
    bool&            onGround
);
 
// Convenience: resolve against every solid in a list.
void ResolveAll(
    Vector2&                      position,
    Vector2&                      velocity,
    float                         w,
    float                         h,
    const std::vector<Rectangle>& solids,
    bool&                         onGround
);
 
