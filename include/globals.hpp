#pragma once
#include <raylib.h>
#include <string>
#include <vector>
#include <set>

namespace Global {
    constexpr int SCREEN_WIDTH  = 800;
    constexpr int SCREEN_HEIGHT = 450;

    inline float deltaTime = 0.0f;

    enum class GameState {
        MENU,
        OPTIONS,   // overlay accessible from MENU or PAUSED
        PLAYING,
        PAUSED,
        GAME_OVER
    };
    inline GameState currentGameState  = GameState::MENU;
    inline GameState previousGameState = GameState::MENU; // for OPTIONS "back"

    // Runtime config (loaded from assets/config.json)
    struct Config {
        float masterVolume = 1.0f;
        float musicVolume  = 0.7f;
        float sfxVolume    = 1.0f;
    };
    inline Config config;

    // Session game data — inventory, unlocked gates, score
    struct GameData {
        std::vector<std::string> inventory;     // key IDs the player is carrying
        std::set<std::string>    unlockedGates;  // gate IDs that have been opened
        int   score       = 0;
        float playtime    = 0.0f;               // seconds since session start
    };
    inline GameData gameData;
}
