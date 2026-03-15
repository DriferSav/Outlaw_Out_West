#include <collision.hpp>
#include <algorithm>

ResolveResult ResolveAABB(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const Rectangle& solid)
{
    ResolveResult r;
    const Rectangle player { position.x, position.y, w, h };
    if (!CheckCollisionRecs(player, solid)) return r;

    const float oLeft   = (player.x + player.width)  - solid.x;
    const float oRight  = (solid.x  + solid.width)   - player.x;
    const float oTop    = (player.y + player.height)  - solid.y;
    const float oBottom = (solid.y  + solid.height)   - player.y;

    const float minX = std::min(oLeft, oRight);
    const float minY = std::min(oTop,  oBottom);

    if (minX < minY) {
        if (oLeft < oRight) { position.x -= oLeft;   velocity.x = 0.0f; r.hitLeft  = true; }
        else                { position.x += oRight;  velocity.x = 0.0f; r.hitRight = true; }
    } else {
        if (oTop < oBottom) { position.y -= oTop;    velocity.y = 0.0f; r.hitTop    = true; }
        else                { position.y += oBottom; if (velocity.y < 0.0f) velocity.y = 0.0f; r.hitBottom = true; }
    }
    return r;
}

bool ResolveOneSided(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const Rectangle& platform,
    float prevBottomY)
{
    // Only collide if falling and previously above the platform
    if (velocity.y <= 0.0f) return false;
    if (prevBottomY > platform.y) return false; // was already below top edge

    const Rectangle player { position.x, position.y, w, h };
    if (!CheckCollisionRecs(player, platform)) return false;

    const float oTop = (player.y + player.height) - platform.y;
    if (oTop <= 0.0f) return false;

    position.y -= oTop;
    velocity.y  = 0.0f;
    return true;
}

void ResolveAll(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const std::vector<Rectangle>& solids,
    bool& onGround,
    bool& touchingWallLeft,
    bool& touchingWallRight)
{
    for (const Rectangle& solid : solids) {
        ResolveResult r = ResolveAABB(position, velocity, w, h, solid);
        if (r.hitTop)   onGround         = true;
        if (r.hitLeft)  touchingWallRight = true; // player came from left, hit right side of player
        if (r.hitRight) touchingWallLeft  = true;
    }
}

void ResolveOneSidedAll(
    Vector2& position, Vector2& velocity,
    float w, float h,
    const std::vector<Rectangle>& platforms,
    float prevBottomY,
    bool  fallThrough,
    bool& onGround)
{
    if (fallThrough) return;
    for (const Rectangle& p : platforms) {
        if (ResolveOneSided(position, velocity, w, h, p, prevBottomY))
            onGround = true;
    }
}
