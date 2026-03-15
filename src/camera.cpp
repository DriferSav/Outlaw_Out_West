#include <camera.hpp>
#include <globals.hpp>
#include <cmath>

GameCamera gameCamera;

static float Lerp (float a, float b, float t) { return a + (b - a) * t; }
static float Clamp(float v, float lo, float hi){ return v < lo ? lo : (v > hi ? hi : v); }

// ---------------------------------------------------------------------------
// Constructor / reset
// ---------------------------------------------------------------------------
GameCamera::GameCamera() {
    Reset();
}

void GameCamera::Reset() {
    // Camera offset is in SCREEN pixels (the point on screen where cam.target lands).
    // We aim for 50% horizontal, 55% vertical (shows more sky above the player).
    // Multiply by scale so the offset tracks the actual window size.
    cam.zoom     = Global::scale;   // scales world so 1 design-px = scale screen-px
    cam.offset.x = Global::letterboxX + Global::DESIGN_W * Global::scale * 0.50f;
    cam.offset.y = Global::letterboxY + Global::DESIGN_H * Global::scale * 0.55f;
    cam.target   = {};
    cam.rotation = 0.0f;
    desired      = {};
    anchorX      = 0.0f;
    lookaheadX   = 0.0f;
    lookaheadDir = 1.0f;
    vertPan      = 0.0f;
}

void GameCamera::SetBounds(float worldWidth, float worldHeight) {
    boundW = worldWidth;
    boundH = worldHeight;
}

// ---------------------------------------------------------------------------
// Update
// All distances here are in DESIGN-SPACE (world coordinates).
// The camera zoom handles the conversion to screen pixels automatically.
// ---------------------------------------------------------------------------
void GameCamera::Update(Vector2 playerPos, float playerW, float playerH,
                        float playerVelX,
                        bool lookingUp, bool lookingDown,
                        float dt)
{
    // Keep zoom and offset current in case the window was resized
    cam.zoom     = Global::scale;
    cam.offset.x = Global::letterboxX + Global::DESIGN_W * Global::scale * 0.50f;
    cam.offset.y = Global::letterboxY + Global::DESIGN_H * Global::scale * 0.55f;

    const float pcx = playerPos.x + playerW * 0.5f;
    const float pcy = playerPos.y + playerH * 0.5f;

    // ---- Horizontal deadzone (design-space pixels) ----
    if (pcx > anchorX + DEADZONE_HALF) anchorX = pcx - DEADZONE_HALF;
    else if (pcx < anchorX - DEADZONE_HALF) anchorX = pcx + DEADZONE_HALF;

    // ---- Lookahead ----
    if      (playerVelX >  20.0f) lookaheadDir =  1.0f;
    else if (playerVelX < -20.0f) lookaheadDir = -1.0f;

    const float laTarget = lookaheadDir * LOOKAHEAD_DIST;
    const float laStep   = LOOKAHEAD_SPEED * dt;
    const float laDelta  = laTarget - lookaheadX;
    lookaheadX += (fabsf(laDelta) < laStep) ? laDelta : (laDelta > 0 ? laStep : -laStep);

    // ---- Vertical look-pan ----
    const float panTarget = lookingUp ? -PAN_DIST : (lookingDown ? PAN_DIST : 0.0f);
    const float panStep   = PAN_SPEED * dt;
    const float panDelta  = panTarget - vertPan;
    vertPan += (fabsf(panDelta) < panStep) ? panDelta : (panDelta > 0 ? panStep : -panStep);

    // ---- Desired world-space camera target ----
    float desiredX = anchorX + lookaheadX;
    float desiredY = pcy     + vertPan;

    // ---- Clamp to world bounds ----
    // cam.offset is in screen-px; divide by zoom to get design-space half-extents
    const float halfW  = cam.offset.x / cam.zoom;
    const float halfHT = cam.offset.y / cam.zoom;
    const float halfHB = (Global::DESIGN_H * 0.45f); // remaining below bias point

    desiredX = Clamp(desiredX, halfW,  boundW - halfW);
    desiredY = Clamp(desiredY, halfHT, boundH - halfHB);

    // ---- Smooth lerp ----
    desired.x = Lerp(desired.x, desiredX, LERP_X * dt);
    desired.y = Lerp(desired.y, desiredY, LERP_Y * dt);
    cam.target = desired;
}

void GameCamera::BeginWorldDraw() const { BeginMode2D(cam); }
void GameCamera::EndWorldDraw()   const { EndMode2D(); }

Vector2 GameCamera::ScreenToWorld(Vector2 screenPos) const {
    return GetScreenToWorld2D(screenPos, cam);
}
