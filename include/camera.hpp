#pragma once
#include <raylib.h>

// ---------------------------------------------------------------------------
// GameCamera — Blasphemous-style smooth follow camera
//
// World positions are always in DESIGN-SPACE (1280x720 units).
// Global::scale is applied as Camera2D zoom so all world drawing
// is automatically scaled to any actual window size.
//
// Call Reset() after changing the window resolution.
// ---------------------------------------------------------------------------
class GameCamera {
public:
    GameCamera();

    // Recompute zoom and offset from current Global::scale.
    // Call after window resize or on level start.
    void Reset();

    // Set world bounds in design-space units (call after every level load).
    void SetBounds(float worldWidth, float worldHeight);

    // Update camera target each frame.
    void Update(Vector2 playerPos, float playerW, float playerH,
                float playerVelX,
                bool lookingUp, bool lookingDown,
                float dt);

    void BeginWorldDraw() const;
    void EndWorldDraw()   const;

    // Convert a screen pixel position to design-space world coordinates.
    Vector2 ScreenToWorld(Vector2 screenPos) const;

    const Camera2D& Raw() const { return cam; }

private:
    Camera2D cam     = {};
    Vector2  desired = {};
    float    anchorX = 0.0f;

    float lookaheadX   = 0.0f;
    float lookaheadDir = 1.0f;
    float vertPan      = 0.0f;

    // All distances in design-space pixels
    static constexpr float DEADZONE_HALF  =  80.0f;
    static constexpr float LOOKAHEAD_DIST = 120.0f;
    static constexpr float LOOKAHEAD_SPEED= 160.0f;
    static constexpr float PAN_DIST       = 200.0f;
    static constexpr float PAN_SPEED      = 240.0f;
    static constexpr float LERP_X         =   5.0f;
    static constexpr float LERP_Y         =   7.0f;

    float boundW = 3840.0f;
    float boundH = 1440.0f;
};

extern GameCamera gameCamera;
