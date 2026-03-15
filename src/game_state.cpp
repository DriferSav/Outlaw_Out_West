#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <player.hpp>

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------
void updateGameState(Global::GameState& currentState) {
    switch (currentState) {
        case Global::GameState::MENU:      updateMenu(currentState);     break;
        case Global::GameState::PLAYING:   updatePlaying(currentState);  break;
        case Global::GameState::PAUSED:    updatePaused(currentState);   break;
        case Global::GameState::GAME_OVER: updateGameOver(currentState); break;
    }
}

void drawGameState(const Global::GameState& currentState) {
    BeginDrawing();
    switch (currentState) {
        case Global::GameState::MENU:      drawMenu();     break;
        case Global::GameState::PLAYING:   drawPlaying();  break;
        case Global::GameState::PAUSED:    drawPaused();   break;
        case Global::GameState::GAME_OVER: drawGameOver(); break;
    }
    EndDrawing();
}

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------
void updateMenu(Global::GameState& state) {
    if (input.IsPressed(Action::ENTER)) {
        bulletPool.Clear();
        level.Load("assets/levels/level_01.json");
        player = Player();
        state  = Global::GameState::PLAYING;
    }
}

void drawMenu() {
    ClearBackground(BLACK);
    const char* title  = "Outlaw Out West";
    const char* prompt = "Press Enter / A to play";
    DrawText(title,  Global::SCREEN_WIDTH / 2 - MeasureText(title,  30) / 2,
             Global::SCREEN_HEIGHT / 2 - 40, 30, GOLD);
    DrawText(prompt, Global::SCREEN_WIDTH / 2 - MeasureText(prompt, 18) / 2,
             Global::SCREEN_HEIGHT / 2 + 10, 18, RAYWHITE);
}

// ---------------------------------------------------------------------------
// Playing
// ---------------------------------------------------------------------------
void updatePlaying(Global::GameState& state) {
    // Pause — PAUSE action covers both ESC (keyboard) and Start (gamepad)
    if (input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PAUSED;
        return;
    }

    bulletPool.Update(Global::deltaTime);

    // Death transition — player.Update() already set isDead in Collide()
    if (player.IsDead()) {
        state = Global::GameState::GAME_OVER;
    }
}

void drawPlaying() {
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();
    bulletPool.Draw();
    player.Draw();

#ifdef GAME_DEBUG
    DrawText(TextFormat("state: env=%d mov=%d act=%d  aim=%.0fdeg  bullets=%d",
        static_cast<int>(player.GetEnvironmentState()),
        static_cast<int>(player.GetMovementState()),
        static_cast<int>(player.GetInteractionState()),
        player.GetAimAngle() * RAD2DEG,
        bulletPool.ActiveCount()),
        8, 8, 14, GREEN);
#endif
}

// ---------------------------------------------------------------------------
// Paused
// ---------------------------------------------------------------------------
void updatePaused(Global::GameState& state) {
    // Same PAUSE action unpauses — ESC or Start, no special case needed
    if (input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PLAYING;
    }
}

void drawPaused() {
    // Draw frozen game world behind overlay
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();
    bulletPool.Draw();
    player.Draw();

    // Dim overlay
    DrawRectangle(0, 0, Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, { 0, 0, 0, 140 });

    const char* title  = "Paused";
    const char* prompt = "ESC / Start to resume";
    DrawText(title,  Global::SCREEN_WIDTH / 2 - MeasureText(title,  28) / 2,
             Global::SCREEN_HEIGHT / 2 - 24, 28, WHITE);
    DrawText(prompt, Global::SCREEN_WIDTH / 2 - MeasureText(prompt, 16) / 2,
             Global::SCREEN_HEIGHT / 2 + 14, 16, LIGHTGRAY);
}

// ---------------------------------------------------------------------------
// Game Over
// ---------------------------------------------------------------------------
void updateGameOver(Global::GameState& state) {
    // Any of: Enter (keyboard), A/Cross (gamepad)
    if (input.IsPressed(Action::ENTER)) {
        bulletPool.Clear();
        level.Load("assets/levels/level_01.json");
        player = Player(); // full reset — isDead cleared by constructor
        state  = Global::GameState::PLAYING;
    }
}

void drawGameOver() {
    ClearBackground({ 10, 5, 5, 255 }); // very dark red background

    const char* title   = "YOU DIED";
    const char* prompt  = "Enter / A to try again";
    DrawText(title,  Global::SCREEN_WIDTH / 2 - MeasureText(title,  48) / 2,
             Global::SCREEN_HEIGHT / 2 - 50, 48, RED);
    DrawText(prompt, Global::SCREEN_WIDTH / 2 - MeasureText(prompt, 18) / 2,
             Global::SCREEN_HEIGHT / 2 + 20, 18, LIGHTGRAY);
}
