#pragma once
#include <raylib.h>
#include <globals.hpp>

void updateGameState(Global::GameState& state);
void drawGameState  (const Global::GameState& state);

// Per-state update + draw
void updateMenu    (Global::GameState& state);
void updateOptions (Global::GameState& state);
void updatePlaying (Global::GameState& state);
void updatePaused  (Global::GameState& state);
void updateGameOver(Global::GameState& state);

void drawMenu();
void drawOptions();
void drawPlaying();
void drawPaused();
void drawGameOver();
