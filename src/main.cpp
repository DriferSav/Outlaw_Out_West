#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <level.hpp>
#include <game_state.hpp>
#include <player.hpp>

int main() {
    InitWindow(Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, "Outlaw Out West");
    SetTargetFPS(60);

    // FIX: Raylib binds ESC to WindowShouldClose() by default.
    // Clearing the exit key lets us handle ESC ourselves via Action::PAUSE.
    SetExitKey(KEY_NULL);

    input.LoadDefaultBindings();

    uint64_t frameId = 0;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        Global::deltaTime = dt;

        input.Update();

        {
            FrameState frame;
            frame.frameId   = frameId++;
            frame.deltaTime = dt;
            frame.input     = input.GetFrame();
            frame.player    = player.GetFrame();
            frameHistory.Push(frame);
        }

        player.Update(dt);
        updateGameState(Global::currentGameState);

        drawGameState(Global::currentGameState);
    }

    CloseWindow();
    return 0;
}
