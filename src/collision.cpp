#include <collision.hpp>
#include <algorithm>
 
void ResolveAABB(
    Vector2&         position,
    Vector2&         velocity,
    float            w,
    float            h,
    const Rectangle& solid,
    bool&            onGround)
{
    // Player's current rect
    const Rectangle player { position.x, position.y, w, h };
 
    // Early-out: Raylib's CheckCollisionRecs is a fast broadphase check
    if (!CheckCollisionRecs(player, solid)) return;
 
    // ---------- penetration depth on each face ----------
    // Positive value = overlapping that face. Negative = not touching.
    //
    // In Raylib, Y increases downward, so:
    //   "top"    of solid = solid.y
    //   "bottom" of solid = solid.y + solid.height
    //
    const float overlapLeft   = (player.x + player.width)  - solid.x;           // player right  into solid left
    const float overlapRight  = (solid.x  + solid.width)   - player.x;          // solid right   into player left
    const float overlapTop    = (player.y + player.height)  - solid.y;           // player bottom into solid top  → landing
    const float overlapBottom = (solid.y  + solid.height)   - player.y;          // solid bottom  into player top → ceiling hit
 
    // Minimum penetration on each axis
    const float minX = std::min(overlapLeft,  overlapRight);
    const float minY = std::min(overlapTop,   overlapBottom);
 
    // ---------- resolve on the axis with less penetration ----------
    if (minX < minY) {
        // --- Horizontal resolution ---
        if (overlapLeft < overlapRight) {
            position.x -= overlapLeft;   // push player left (came from left)
        } else {
            position.x += overlapRight;  // push player right (came from right)
        }
        velocity.x = 0.0f;
 
    } else {
        // --- Vertical resolution ---
        if (overlapTop < overlapBottom) {
            // Player's bottom entered solid's top → player landed on platform
            position.y -= overlapTop;
            velocity.y  = 0.0f;
            onGround    = true;          // caller must clear this before calling ResolveAll
        } else {
            // Player's top entered solid's bottom → player hit a ceiling
            position.y += overlapBottom;
            if (velocity.y < 0.0f) velocity.y = 0.0f;
        }
    }
}
 
void ResolveAll(
    Vector2&                      position,
    Vector2&                      velocity,
    float                         w,
    float                         h,
    const std::vector<Rectangle>& solids,
    bool&                         onGround)
{
    for (const Rectangle& solid : solids) {
        ResolveAABB(position, velocity, w, h, solid, onGround);
    }
}
 
