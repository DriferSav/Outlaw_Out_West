#include <raylib.h>
#include <globals.hpp>
#include <config.hpp>
#include <input.hpp>
#include <frame.hpp>
#include <level.hpp>
#include <camera.hpp>
#include <game_state.hpp>
#include <player.hpp>
#include <audio.hpp>
#include <cstdlib>  // atoi

// ---------------------------------------------------------------------------
// Usage: ./Outlaw [width] [height]
//   ./Outlaw              — uses saved config or defaults (1280x720)
//   ./Outlaw 1920 1080    — run at 1920x1080
//   ./Outlaw 2560 1440    — run at 2560x1440
//   ./Outlaw 800  600     — run at 800x600  (letterboxed, 4:3)
// ---------------------------------------------------------------------------
int main(int argc, char** argv) {

    // 1. Load config first so we have saved window size
    LoadConfig("assets/config.json");

    // 2. Command-line overrides saved config
    if (argc >= 3) {
        Global::config.windowWidth  = atoi(argv[1]);
        Global::config.windowHeight = atoi(argv[2]);
    }
    // Clamp to sane minimum
    if (Global::config.windowWidth  < 320) Global::config.windowWidth  = 320;
    if (Global::config.windowHeight < 180) Global::config.windowHeight = 180;

    // 3. Apply window size to Global
    Global::SCREEN_WIDTH  = Global::config.windowWidth;
    Global::SCREEN_HEIGHT = Global::config.windowHeight;

    // 4. Compute scale factor and letterbox offsets BEFORE creating the window
    Global::ComputeScale();

    // 5. Create window and camera
    InitWindow(Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, "Outlaw Out West");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    input.LoadDefaultBindings();
    gameCamera.Reset();
    audio.Init();
    audio.PlayMusic(MusicId::MENU); // sets cam.zoom = Global::scale

    uint64_t frameId = 0;

    while (!WindowShouldClose() && !Global::shouldExit) {
        const float dt = GetFrameTime();
        Global::deltaTime = dt;

        input.Update();
        audio.Update(dt);

        // Snapshot BEFORE update
        {
            FrameState fs;
            fs.frameId   = frameId++;
            fs.deltaTime = dt;
            fs.input     = input.GetFrame();
            fs.player    = player.GetFrame();
            frameHistory.Push(fs);
        }

        if (Global::currentGameState == Global::GameState::PLAYING) {
            player.Update(dt);
            Global::gameData.playtime += dt;

            gameCamera.Update(
                player.GetPosition(),
                32.0f, 48.0f,
                player.GetVelocityX(),
                input.IsHeld(Action::MOVE_UP),
                input.IsHeld(Action::MOVE_DOWN),
                dt
            );
        }

        updateGameState(Global::currentGameState);
        drawGameState  (Global::currentGameState);
    }

    SaveConfig("assets/config.json");
    audio.Shutdown();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
