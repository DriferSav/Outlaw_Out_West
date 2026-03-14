#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <player.hpp>

int main() {
    InitWindow(Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, "Outlaw Out West");

    SetTargetFPS(60);
    
    input.LoadDefaultBindings();

    while (!WindowShouldClose()) {
        // Get Delta Time
        float dt = GetFrameTime();

        // Update
        input.Update();

        player.Update(dt);

        updateGameState(Global::currentGameState);

        // Draw
        drawGameState(Global::currentGameState);
    }

    CloseWindow();
    return 0;
}
