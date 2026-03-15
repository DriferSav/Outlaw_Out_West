#include <raylib.h>
#include <globals.hpp>
#include <config.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <level.hpp>
#include <game_state.hpp>
#include <player.hpp>

int main() {
    InitWindow(Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, "Outlaw Out West");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL); // ESC handled by Action::PAUSE, not Raylib

    LoadConfig("assets/config.json");
    input.LoadDefaultBindings();

    uint64_t frameId = 0;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        Global::deltaTime = dt;

        input.Update();

        // Snapshot BEFORE update
        {
            FrameState fs;
            fs.frameId   = frameId++;
            fs.deltaTime = dt;
            fs.input     = input.GetFrame();
            fs.player    = player.GetFrame();
            frameHistory.Push(fs);
        }

        // Player physics update only while actively playing
        if (Global::currentGameState == Global::GameState::PLAYING) {
            player.Update(dt);
            Global::gameData.playtime += dt;
        }

        updateGameState(Global::currentGameState);
        drawGameState  (Global::currentGameState);
    }

    SaveConfig("assets/config.json");
    CloseWindow();
    return 0;
}
