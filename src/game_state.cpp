#include <raylib.h>
#include <globals.hpp>
#include <config.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <enemy.hpp>
#include <hud.hpp>
#include <player.hpp>
#include <cmath>

// ---------------------------------------------------------------------------
// Simple menu cursor helper — shared between MENU and PAUSE
// ---------------------------------------------------------------------------
namespace {
    // Move an int index with UP/DOWN, wrap around itemCount
    int MoveCursor(int cur, int itemCount) {
        if (input.IsPressed(Action::MOVE_UP)   ||
            (input.GetLeftStick().y < -0.5f && input.IsPressed(Action::MOVE_UP)))
            cur = (cur - 1 + itemCount) % itemCount;
        if (input.IsPressed(Action::MOVE_DOWN) ||
            (input.GetLeftStick().y >  0.5f && input.IsPressed(Action::MOVE_DOWN)))
            cur = (cur + 1) % itemCount;
        return cur;
    }

    // Draw a vertical list of options, highlighting selectedIdx
    void DrawMenuList(const char** items, int count, int selected,
                      int cx, int startY, int spacing, int fontSize)
    {
        for (int i = 0; i < count; i++) {
            Color c = (i == selected) ? GOLD : Color{200,200,200,220};
            int   w = MeasureText(items[i], fontSize);
            DrawText(items[i], cx - w/2, startY + i*spacing, fontSize, c);
            if (i == selected) {
                DrawText(">", cx - w/2 - 20, startY + i*spacing, fontSize, GOLD);
            }
        }
    }

    int menuCursor  = 0; // MENU selected item
    int pauseCursor = 0; // PAUSE selected item
    int optCursor   = 0; // OPTIONS selected item

    void StartPlaying() {
        bulletPool.Clear();
        enemyManager.Clear();
        level.Load("assets/levels/level_01.json");
        player = Player();
        // Use spawn from level
        // (Player constructor sets default; level overrides via GetSpawn)
        enemyManager.SpawnFromLevel();
        Global::gameData = Global::GameData{};
    }
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------
void updateGameState(Global::GameState& state) {
    switch (state) {
        case Global::GameState::MENU:      updateMenu    (state); break;
        case Global::GameState::OPTIONS:   updateOptions (state); break;
        case Global::GameState::PLAYING:   updatePlaying (state); break;
        case Global::GameState::PAUSED:    updatePaused  (state); break;
        case Global::GameState::GAME_OVER: updateGameOver(state); break;
    }
}

void drawGameState(const Global::GameState& state) {
    BeginDrawing();
    switch (state) {
        case Global::GameState::MENU:      drawMenu();     break;
        case Global::GameState::OPTIONS:   drawOptions();  break;
        case Global::GameState::PLAYING:   drawPlaying();  break;
        case Global::GameState::PAUSED:    drawPaused();   break;
        case Global::GameState::GAME_OVER: drawGameOver(); break;
    }
    EndDrawing();
}

// ---------------------------------------------------------------------------
// MENU
// Items: Play | Load | Options | Exit
// ---------------------------------------------------------------------------
static const char* MENU_ITEMS[] = { "Play", "Load", "Options", "Exit" };
static constexpr int MENU_COUNT = 4;

void updateMenu(Global::GameState& state) {
    menuCursor = MoveCursor(menuCursor, MENU_COUNT);
    if (input.IsPressed(Action::ENTER)) {
        switch (menuCursor) {
            case 0: StartPlaying(); state = Global::GameState::PLAYING; break;
            case 1: /* TODO: open file picker / slot select */ break;
            case 2:
                Global::previousGameState = Global::GameState::MENU;
                state = Global::GameState::OPTIONS;
                break;
            case 3: CloseWindow(); break;
        }
    }
}

void drawMenu() {
    ClearBackground(BLACK);
    // Title
    const char* title = "Outlaw Out West";
    DrawText(title,
        Global::SCREEN_WIDTH/2 - MeasureText(title, 36)/2,
        Global::SCREEN_HEIGHT/2 - 100, 36, GOLD);

    DrawMenuList(MENU_ITEMS, MENU_COUNT, menuCursor,
        Global::SCREEN_WIDTH/2,
        Global::SCREEN_HEIGHT/2 - 20, 36, 22);

    // Controls hint
    DrawText("Arrow keys / D-pad to navigate   Enter / A to select",
        Global::SCREEN_WIDTH/2 - MeasureText("Arrow keys / D-pad to navigate   Enter / A to select", 12)/2,
        Global::SCREEN_HEIGHT - 20, 12, DARKGRAY);
}

// ---------------------------------------------------------------------------
// OPTIONS
// Items: Master Vol | Music Vol | SFX Vol | Back
// ---------------------------------------------------------------------------
static const char* OPT_ITEMS[] = {
    "Master Volume", "Music Volume", "SFX Volume", "Back"
};
static constexpr int OPT_COUNT = 4;

static void DrawVolumeSlider(const char* label, float value, int cx, int y) {
    DrawText(label, cx - 140, y, 18, WHITE);
    DrawRectangle(cx - 10, y + 4, 120, 10, DARKGRAY);
    DrawRectangle(cx - 10, y + 4, (int)(120 * value), 10, GOLD);
    DrawRectangleLinesEx({(float)(cx-10),(float)(y+4),120,10}, 1, GRAY);
}

void updateOptions(Global::GameState& state) {
    optCursor = MoveCursor(optCursor, OPT_COUNT);

    // LEFT/RIGHT (or stick X) adjusts sliders
    float delta = 0.0f;
    if (input.IsPressed(Action::MOVE_LEFT))  delta = -0.05f;
    if (input.IsPressed(Action::MOVE_RIGHT)) delta =  0.05f;
    const float sx = input.GetLeftStick().x;
    if (fabsf(sx) > 0.3f) delta = sx * 0.02f;

    auto& c = Global::config;
    if (delta != 0.0f) {
        switch (optCursor) {
            case 0: c.masterVolume = std::fmax(0.f, std::fmin(1.f, c.masterVolume + delta)); break;
            case 1: c.musicVolume  = std::fmax(0.f, std::fmin(1.f, c.musicVolume  + delta)); break;
            case 2: c.sfxVolume    = std::fmax(0.f, std::fmin(1.f, c.sfxVolume    + delta)); break;
        }
        SaveConfig("assets/config.json");
    }

    // Back / Escape
    if (input.IsPressed(Action::PAUSE) ||
        (optCursor == 3 && input.IsPressed(Action::ENTER))) {
        state = Global::previousGameState;
    }
}

void drawOptions() {
    ClearBackground({ 15, 15, 25, 255 });
    const char* title = "Options";
    DrawText(title,
        Global::SCREEN_WIDTH/2 - MeasureText(title, 28)/2,
        50, 28, GOLD);

    const auto& c = Global::config;
    int cy = 130, cx = Global::SCREEN_WIDTH/2;
    DrawVolumeSlider("Master Volume", c.masterVolume, cx, cy);      cy += 50;
    DrawVolumeSlider("Music Volume",  c.musicVolume,  cx, cy);      cy += 50;
    DrawVolumeSlider("SFX Volume",    c.sfxVolume,    cx, cy);      cy += 50;

    // Highlight current slider row
    int rowY = 130 + optCursor * 50;
    DrawRectangle(cx - 155, rowY - 2, 280, 26, {255,255,255,18});

    // Back button
    Color backC = (optCursor == 3) ? GOLD : Color{200,200,200,220};
    int backW   = MeasureText("Back", 22);
    DrawText("Back", cx - backW/2, cy + 10, 22, backC);
    if (optCursor == 3) DrawText(">", cx - backW/2 - 20, cy + 10, 22, GOLD);

    DrawText("Left/Right to adjust   ESC to go back",
        cx - MeasureText("Left/Right to adjust   ESC to go back", 12)/2,
        Global::SCREEN_HEIGHT - 20, 12, DARKGRAY);
}

// ---------------------------------------------------------------------------
// PLAYING
// ---------------------------------------------------------------------------
void updatePlaying(Global::GameState& state) {
    if (input.IsPressed(Action::PAUSE)) {
        pauseCursor = 0;
        Global::previousGameState = Global::GameState::PLAYING;
        state = Global::GameState::PAUSED;
        return;
    }
    // NOTE: player.Update() is called in main.cpp before updateGameState,
    // but ONLY when state == PLAYING (gated in main).
    int enemyBulletsHit = 0;
    Rectangle pb = { player.GetFrame().position.x,
                     player.GetFrame().position.y, 32, 48 };
    bulletPool.Update(Global::deltaTime, pb, enemyBulletsHit);
    if (enemyBulletsHit > 0) player.TakeDamage(enemyBulletsHit);

    enemyManager.Update(Global::deltaTime);

    // Check for death transition
    if (player.IsDead()) state = Global::GameState::GAME_OVER;
}

void drawPlaying() {
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();
    bulletPool.Draw();
    enemyManager.Draw();
    player.Draw();
    DrawHUD();

#ifdef GAME_DEBUG
    DrawText(TextFormat("mov=%d env=%d act=%d ammo=%d/%d reload=%.1f",
        (int)player.GetMovementState(),
        (int)player.GetEnvironmentState(),
        (int)player.GetInteractionState(),
        player.GetAmmo(), player.GetMaxAmmo(),
        player.IsReloading() ? player.GetReloadProgress() : 0.f),
        8, Global::SCREEN_HEIGHT - 20, 12, GREEN);
#endif
}

// ---------------------------------------------------------------------------
// PAUSED
// Items: Resume | Options | Exit to Menu
// ---------------------------------------------------------------------------
static const char* PAUSE_ITEMS[] = { "Resume", "Options", "Exit to Menu" };
static constexpr int PAUSE_COUNT = 3;

void updatePaused(Global::GameState& state) {
    // IMPORTANT: nothing in game world updates here — pause is a true freeze
    pauseCursor = MoveCursor(pauseCursor, PAUSE_COUNT);

    if (input.IsPressed(Action::PAUSE)) {
        // ESC always resumes
        state = Global::GameState::PLAYING;
        return;
    }

    if (input.IsPressed(Action::ENTER)) {
        switch (pauseCursor) {
            case 0: state = Global::GameState::PLAYING; break;
            case 1:
                Global::previousGameState = Global::GameState::PAUSED;
                state = Global::GameState::OPTIONS;
                break;
            case 2:
                // Reset session and return to main menu
                bulletPool.Clear();
                enemyManager.Clear();
                level.Unload();
                Global::gameData = Global::GameData{};
                menuCursor = 0;
                state = Global::GameState::MENU;
                break;
        }
    }
}

void drawPaused() {
    // Draw frozen world beneath overlay
    ClearBackground({ 30, 30, 46, 255 });
    level.Draw();
    bulletPool.Draw();
    enemyManager.Draw();
    player.Draw();
    DrawHUD();

    // Dim overlay
    DrawRectangle(0, 0, Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, {0,0,0,150});

    const char* title = "Paused";
    DrawText(title,
        Global::SCREEN_WIDTH/2 - MeasureText(title, 30)/2,
        Global::SCREEN_HEIGHT/2 - 80, 30, WHITE);

    DrawMenuList(PAUSE_ITEMS, PAUSE_COUNT, pauseCursor,
        Global::SCREEN_WIDTH/2,
        Global::SCREEN_HEIGHT/2 - 20, 38, 22);

    DrawText("ESC to resume",
        Global::SCREEN_WIDTH/2 - MeasureText("ESC to resume", 12)/2,
        Global::SCREEN_HEIGHT - 20, 12, DARKGRAY);
}

// ---------------------------------------------------------------------------
// GAME OVER
// ---------------------------------------------------------------------------
void updateGameOver(Global::GameState& state) {
    if (input.IsPressed(Action::ENTER)) {
        StartPlaying();
        state = Global::GameState::PLAYING;
    }
    if (input.IsPressed(Action::PAUSE)) {
        menuCursor = 0;
        state = Global::GameState::MENU;
    }
}

void drawGameOver() {
    ClearBackground({ 10, 5, 5, 255 });
    const char* title  = "YOU DIED";
    const char* retry  = "Enter / A  —  Try Again";
    const char* menu   = "ESC        —  Main Menu";
    DrawText(title,
        Global::SCREEN_WIDTH/2 - MeasureText(title, 52)/2,
        Global::SCREEN_HEIGHT/2 - 60, 52, RED);
    DrawText(retry,
        Global::SCREEN_WIDTH/2 - MeasureText(retry, 18)/2,
        Global::SCREEN_HEIGHT/2 + 20, 18, LIGHTGRAY);
    DrawText(menu,
        Global::SCREEN_WIDTH/2 - MeasureText(menu,  18)/2,
        Global::SCREEN_HEIGHT/2 + 48, 18, DARKGRAY);
}
