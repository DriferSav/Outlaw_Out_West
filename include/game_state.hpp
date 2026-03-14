#pragma once
#include <raylib.h>
#include <globals.hpp>

void updateGameState(Global::GameState& currentState);
void drawGameState(const Global::GameState& currentState);

void updateMenu(Global::GameState& state);
void updatePlaying(Global::GameState& state);
void updatePaused(Global::GameState& state);

void drawMenu();
void drawPlaying();
void drawPaused();
