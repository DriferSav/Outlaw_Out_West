#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <player.hpp>
 
void updateGameState(Global::GameState& currentState) {
    switch(currentState) {
        case Global::GameState::MENU:      updateMenu(currentState);    break;
        case Global::GameState::PLAYING:   updatePlaying(currentState); break;
        case Global::GameState::PAUSED:    updatePaused(currentState);  break;
        case Global::GameState::GAME_OVER:                              break;
    }
}
 
void drawGameState(const Global::GameState& currentState) {
    BeginDrawing();
    switch(currentState) {
        case Global::GameState::MENU:      drawMenu();    break;
        case Global::GameState::PLAYING:   drawPlaying(); break;
        case Global::GameState::PAUSED:    drawPaused();  break;
        case Global::GameState::GAME_OVER:
            ClearBackground(BLACK);
            DrawText("Game Over", 400, 225, 20, WHITE);
            break;
    }
    EndDrawing();
}
 
// ---------------------------------------------------------------------------
// State updates
// ---------------------------------------------------------------------------
void updateMenu(Global::GameState& state) {
    if (input.IsPressed(Action::ENTER)) {
        if (level.Load("assets/levels/level_01.json")) {
            bulletPool.Clear();
            player = Player();
        }
        state = Global::GameState::PLAYING;
    }
}
 
void updatePlaying(Global::GameState& state) {
    if (input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PAUSED;
        return;
    }
    bulletPool.Update(Global::deltaTime);
}
 
void updatePaused(Global::GameState& state) {
    if (input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PLAYING;
    }
    // bullets do NOT update while paused — intentional
}
 
// ---------------------------------------------------------------------------
// Draw functions
// ---------------------------------------------------------------------------
void drawMenu() {
    ClearBackground(BLACK);
    DrawText("Press Enter to start",
        Global::SCREEN_WIDTH / 2 - MeasureText("Press Enter to start", 20) / 2,
        Global::SCREEN_HEIGHT / 2, 20, RAYWHITE);
}
 
void drawPlaying() {
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();       // platforms
    bulletPool.Draw();  // bullets behind player
    player.Draw();      // player on top
 
#ifdef GAME_DEBUG
    DrawText(TextFormat("bullets: %d  aim: %.1f deg",
        bulletPool.ActiveCount(),
        player.GetAimAngle() * RAD2DEG), 8, 8, 16, GREEN);
#endif
}
 
void drawPaused() {
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();
    bulletPool.Draw();
    player.Draw();
    DrawRectangle(0, 0, Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, { 0, 0, 0, 120 });
    DrawText("Paused — press ESC to resume",
        Global::SCREEN_WIDTH / 2 - MeasureText("Paused — press ESC to resume", 20) / 2,
        Global::SCREEN_HEIGHT / 2, 20, WHITE);
}
 
