#pragma once
#include <raylib.h>
#include <string>
#include <vector>
#include <set>

namespace Global {
    // -------------------------------------------------------------------------
    // Design resolution — all positions and sizes are authored at this size.
    // Never change these. They define the coordinate space of levels, HUD, etc.
    // -------------------------------------------------------------------------
    constexpr float DESIGN_W = 1280.0f;
    constexpr float DESIGN_H =  720.0f;

    // -------------------------------------------------------------------------
    // Actual window size — set from command-line args (or defaults below).
    // Read these anywhere you need real pixel dimensions (e.g. window creation,
    // letterbox bars). Never use these for game-logic positions.
    // -------------------------------------------------------------------------
    inline int SCREEN_WIDTH  = 1280;
    inline int SCREEN_HEIGHT =  720;

    // -------------------------------------------------------------------------
    // Scale factor: actual / design.
    // Computed once in main() after the window size is known.
    // Use this to convert design-space pixel sizes to screen pixels for HUD
    // drawing (DrawText font sizes, rectangle widths, etc.).
    // Camera zoom is set to this value so the world draws at the right size.
    // -------------------------------------------------------------------------
    inline float scale = 1.0f;

    // Horizontal and vertical letterbox offsets (pixels) when aspect ratio
    // doesn't match 16:9. Both are 0 when running at a 16:9 resolution.
    inline float letterboxX = 0.0f;
    inline float letterboxY = 0.0f;

    // Call once in main() after setting SCREEN_WIDTH / SCREEN_HEIGHT.
    // Computes scale and letterbox offsets, and repositions the camera offset.
    inline void ComputeScale() {
        const float sx = (float)SCREEN_WIDTH  / DESIGN_W;
        const float sy = (float)SCREEN_HEIGHT / DESIGN_H;
        scale      = (sx < sy) ? sx : sy;          // fit — use the smaller axis
        letterboxX = ((float)SCREEN_WIDTH  - DESIGN_W * scale) * 0.5f;
        letterboxY = ((float)SCREEN_HEIGHT - DESIGN_H * scale) * 0.5f;
    }

    // Convert a design-space coordinate to a screen pixel.
    // Use for HUD positions that need to be in screen space.
    inline float DS(float designPx) { return designPx * scale; }

    inline float deltaTime = 0.0f;

    enum class GameState {
        MENU,
        OPTIONS,
        PLAYING,
        PAUSED,
        WIN,
        GAME_OVER
    };
    inline GameState currentGameState  = GameState::MENU;
    inline GameState previousGameState = GameState::MENU;
    inline bool      shouldExit        = false;

    struct Config {
        float masterVolume = 1.0f;
        float musicVolume  = 0.7f;
        float sfxVolume    = 1.0f;
        int   windowWidth  = 1280;
        int   windowHeight =  720;
    };
    inline Config config;

    struct GameData {
        std::vector<std::string> inventory;
        std::set<std::string>    unlockedGates;
        int   score    = 0;
        float playtime = 0.0f;
    };
    inline GameData gameData;
}
