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
 
    input.LoadDefaultBindings();
 
    uint64_t frameId = 0;
 
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        Global::deltaTime = dt;
 
        // 1. Poll OS for new key states
        input.Update();
 
        // 2. Snapshot state BEFORE this frame's update.
        //    player.GetFrame() captures position/velocity/onGround from the
        //    END of last frame — exactly what history queries need to look back at.
        {
            FrameState frame;
            frame.frameId   = frameId++;
            frame.deltaTime = dt;
            frame.input     = input.GetFrame();
            frame.player    = player.GetFrame();
            frameHistory.Push(frame);
        }
 
        // 3. Update — player.Update() can now call frameHistory.Get(N)
        player.Update(dt);
        updateGameState(Global::currentGameState);
 
        // 4. Draw
        drawGameState(Global::currentGameState);
    }
 
    CloseWindow();
    return 0;
}
 
