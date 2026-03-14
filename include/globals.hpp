#pragma once
#include <raylib.h>

namespace Global {
    constexpr int SCREEN_WIDTH = 800;
    constexpr int SCREEN_HEIGHT = 450;

    inline float deltaTime = 0.0f;

    enum class GameState {
        MENU,
        PLAYING,
        PAUSED,
        GAME_OVER
    };
    inline GameState currentGameState = GameState::MENU;
}
