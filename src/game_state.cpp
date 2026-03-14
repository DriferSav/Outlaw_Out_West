#include <raylib.h>
#include <globals.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <player.hpp>

void updateGameState(Global::GameState& currentState) {
    switch(currentState) {
        case Global::GameState::MENU:
            updateMenu(currentState);
            break;
        case Global::GameState::PLAYING:
            updatePlaying(currentState);
            break;
        case Global::GameState::PAUSED:
            updatePaused(currentState);
            break;
        case Global::GameState::GAME_OVER:
            break;
    }
}

void drawGameState(const Global::GameState& currentState) {
    BeginDrawing();
    
    switch(currentState) {
        case Global::GameState::MENU:
            drawMenu();
            break;
        case Global::GameState::PLAYING:
            drawPlaying();
            break;
        case Global::GameState::PAUSED:
            drawPaused();
            break;
        case Global::GameState::GAME_OVER:
            DrawText("Game Over", 400, 225, 20, WHITE);
            break;
    };
    
    EndDrawing();
}

void updateMenu(Global::GameState& state) {
    if(input.IsPressed(Action::ENTER)) {
        state = Global::GameState::PLAYING;
    }    
}

void updatePlaying(Global::GameState& state) {
    if(input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PAUSED;
    }
}

void updatePaused(Global::GameState& state) {
    if(input.IsPressed(Action::PAUSE)) {
        state = Global::GameState::PLAYING;
    }
}



void drawMenu() {
    ClearBackground(BLACK);
    DrawText("Press Enter to start", 400, 225, 20, RAYWHITE);
}

void drawPlaying() {
    ClearBackground(RAYWHITE);
    player.Draw();
}

void drawPaused() {
    DrawText("Game Paused", 400, 225, 20, BLACK);
}
