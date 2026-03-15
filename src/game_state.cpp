#include <raylib.h>
#include <globals.hpp>
#include <config.hpp>
#include <input.hpp>
#include <game_state.hpp>
#include <level.hpp>
#include <bullet.hpp>
#include <enemy.hpp>
#include <hud.hpp>
#include <camera.hpp>
#include <audio.hpp>
#include <debug.hpp>
#include <player.hpp>
#include <cmath>
#include <algorithm>

// ---------------------------------------------------------------------------
// UI helpers — all positions in design space (1280x720),
// converted to screen pixels via Global::scale + letterbox offset
// ---------------------------------------------------------------------------
namespace {
    // Scale a design-space X to screen X (int)
    inline int SX(float x) { return (int)(Global::letterboxX + x * Global::scale); }
    inline int SY(float y) { return (int)(Global::letterboxY + y * Global::scale); }
    inline int SF(int  fs) { return (int)(fs  * Global::scale); }
    inline int SW(float w) { return (int)(w   * Global::scale); }

    // Horizontally centre text given design-space centreX
    inline int CentreText(float designCX, const char* text, int fontSize) {
        return SX(designCX) - MeasureText(text, fontSize) / 2;
    }

    // ---------------------------------------------------------------------------
    // Menu cursor navigation
    // ---------------------------------------------------------------------------
    int MoveCursor(int cur, int itemCount) {
        if (input.IsPressed(Action::MOVE_UP))
            cur = (cur - 1 + itemCount) % itemCount;
        if (input.IsPressed(Action::MOVE_DOWN))
            cur = (cur + 1) % itemCount;
        return cur;
    }

    void DrawMenuList(const char** items, int count, int selected,
                      float designCX, float designStartY,
                      float designSpacing, int designFontSize)
    {
        for (int i = 0; i < count; i++) {
            int fs   = SF(designFontSize);
            Color c  = (i == selected) ? GOLD : Color{200,200,200,220};
            int   tw = MeasureText(items[i], fs);
            int   tx = SX(designCX) - tw/2;
            int   ty = SY(designStartY + i * designSpacing);
            DrawText(items[i], tx, ty, fs, c);
            if (i == selected)
                DrawText(">", tx - SW(22), ty, fs, GOLD);
        }
    }

    // ---------------------------------------------------------------------------
    // Shared state
    // ---------------------------------------------------------------------------
    int menuCursor  = 0;
    int pauseCursor = 0;
    int optCursor   = 0;

    void StartPlaying() {
        bulletPool.Clear();
        enemyManager.Clear();
        level.Load("assets/levels/showcase_level.json");
        player = Player();
        player.SetPosition(level.GetSpawn());
        enemyManager.SpawnFromLevel();
        Global::gameData = Global::GameData{};
        Vector2 ws = level.GetWorldSize();
        gameCamera.SetBounds(ws.x, ws.y);
        gameCamera.Reset();
        audio.PlayMusic(MusicId::GAMEPLAY);
    }
}

// ---------------------------------------------------------------------------
// Dispatch
// ---------------------------------------------------------------------------
void updateGameState(Global::GameState& state) {
    DEBUG_UPDATE();
    switch (state) {
        case Global::GameState::MENU:      updateMenu    (state); break;
        case Global::GameState::OPTIONS:   updateOptions (state); break;
        case Global::GameState::PLAYING:   updatePlaying (state); break;
        case Global::GameState::PAUSED:    updatePaused  (state); break;
        case Global::GameState::WIN:       updateWin     (state); break;
        case Global::GameState::GAME_OVER: updateGameOver(state); break;
    }
}

void drawGameState(const Global::GameState& state) {
    BeginDrawing();

    // Letterbox bars (fill areas outside the design viewport)
    if (Global::letterboxX > 0) {
        DrawRectangle(0, 0, (int)Global::letterboxX, Global::SCREEN_HEIGHT, BLACK);
        DrawRectangle(Global::SCREEN_WIDTH - (int)Global::letterboxX, 0,
                      (int)Global::letterboxX + 1, Global::SCREEN_HEIGHT, BLACK);
    }
    if (Global::letterboxY > 0) {
        DrawRectangle(0, 0, Global::SCREEN_WIDTH, (int)Global::letterboxY, BLACK);
        DrawRectangle(0, Global::SCREEN_HEIGHT - (int)Global::letterboxY,
                      Global::SCREEN_WIDTH, (int)Global::letterboxY + 1, BLACK);
    }

    switch (state) {
        case Global::GameState::MENU:      drawMenu();     break;
        case Global::GameState::OPTIONS:   drawOptions();  break;
        case Global::GameState::PLAYING:   drawPlaying();  break;
        case Global::GameState::PAUSED:    drawPaused();   break;
        case Global::GameState::WIN:       drawWin();      break;
        case Global::GameState::GAME_OVER: drawGameOver(); break;
    }

    EndDrawing();
}

// ---------------------------------------------------------------------------
// MENU  —  Play | Load | Options | Exit
// ---------------------------------------------------------------------------
static const char* MENU_ITEMS[] = { "Play", "Load", "Options", "Exit" };
static constexpr int MENU_COUNT = 4;

void updateMenu(Global::GameState& state) {
    menuCursor = MoveCursor(menuCursor, MENU_COUNT);
    audio.PlayMusic(MusicId::MENU);
    if (input.IsPressed(Action::ENTER)) {
        switch (menuCursor) {
            case 0: StartPlaying(); state = Global::GameState::PLAYING; break;
            case 1: break; // TODO: load save
            case 2:
                Global::previousGameState = Global::GameState::MENU;
                state = Global::GameState::OPTIONS;
                break;
            case 3: Global::shouldExit = true; break;
        }
    }
}

void drawMenu() {
    ClearBackground(BLACK);
    const char* title = "Outlaw Out West";
    int tfs = SF(52);
    DrawText(title, CentreText(640, title, tfs), SY(160), tfs, GOLD);
    DrawMenuList(MENU_ITEMS, MENU_COUNT, menuCursor,
        640, 300, 52, 28);
    const char* hint = "Arrow keys / D-pad to navigate   Enter / A to select";
    int hfs = SF(14);
    DrawText(hint, CentreText(640, hint, hfs), SY(694), hfs, DARKGRAY);
}

// ---------------------------------------------------------------------------
// OPTIONS
// ---------------------------------------------------------------------------
static const char* OPT_ITEMS[] = { "Master Volume", "Music Volume", "SFX Volume", "Back" };
static constexpr int OPT_COUNT = 4;

static void DrawVolumeSlider(const char* label, float value,
                              float designCX, float designY) {
    int lfs = SF(20);
    DrawText(label, SX(designCX - 180), SY(designY), lfs, WHITE);
    float barW = 160, barH = 14;
    DrawRectangle(SX(designCX - 10), SY(designY + 6),
                  SW(barW * value), SW(barH), GOLD);
    DrawRectangleLinesEx({ (float)SX(designCX-10), (float)SY(designY+6),
                           (float)SW(barW), (float)SW(barH) }, 1, GRAY);
}

void updateOptions(Global::GameState& state) {
    optCursor = MoveCursor(optCursor, OPT_COUNT);
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
        audio.RefreshVolumes();
    }
    if (input.IsPressed(Action::PAUSE) ||
        (optCursor == 3 && input.IsPressed(Action::ENTER)))
        state = Global::previousGameState;
}

void drawOptions() {
    ClearBackground({ 15, 15, 25, 255 });
    const char* title = "Options";
    int tfs = SF(34);
    DrawText(title, CentreText(640, title, tfs), SY(70), tfs, GOLD);
    const auto& c = Global::config;
    DrawVolumeSlider("Master Volume", c.masterVolume, 640, 210);
    DrawVolumeSlider("Music Volume",  c.musicVolume,  640, 280);
    DrawVolumeSlider("SFX Volume",    c.sfxVolume,    640, 350);
    // Row highlight
    float rowY = 210 + optCursor * 70;
    DrawRectangle(SX(460), SY(rowY - 4), SW(360), SW(36), {255,255,255,18});
    // Back
    Color backC = (optCursor == 3) ? GOLD : Color{200,200,200,220};
    const char* back = "Back";
    int bfs = SF(26);
    DrawText(back, CentreText(640, back, bfs), SY(440), bfs, backC);
    if (optCursor == 3) DrawText(">", CentreText(640, back, bfs) - SW(22), SY(440), bfs, GOLD);
    const char* hint = "Left/Right to adjust   ESC to go back";
    int hfs = SF(14);
    DrawText(hint, CentreText(640, hint, hfs), SY(694), hfs, DARKGRAY);
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
    int enemyBulletsHit = 0;
    Rectangle pb = { player.GetPosition().x, player.GetPosition().y, 32.0f, 48.0f };
    bulletPool.Update(Global::deltaTime, pb, enemyBulletsHit);
    if (enemyBulletsHit > 0) player.TakeDamage(enemyBulletsHit);
    enemyManager.Update(Global::deltaTime);
    if (player.IsDead()) {
        audio.StopMusic();
        audio.PlaySFX(SoundId::GAME_OVER);
        state = Global::GameState::GAME_OVER;
    }
    if (player.HasWon()) {
        audio.StopMusic();
        audio.PlaySFX(SoundId::WIN);
        state = Global::GameState::WIN;
    }
}

void drawPlaying() {
    ClearBackground({ 30, 30, 46, 255 });
    gameCamera.BeginWorldDraw();
    level.Draw();
    bulletPool.Draw();
    enemyManager.Draw();
    player.Draw();
    DEBUG_DRAW_WORLD();
    gameCamera.EndWorldDraw();
    DrawHUD();
    DEBUG_DRAW_SCREEN();
}

// ---------------------------------------------------------------------------
// PAUSED  —  Resume | Options | Exit to Menu
// ---------------------------------------------------------------------------
static const char* PAUSE_ITEMS[] = { "Resume", "Options", "Exit to Menu" };
static constexpr int PAUSE_COUNT = 3;

void updatePaused(Global::GameState& state) {
    pauseCursor = MoveCursor(pauseCursor, PAUSE_COUNT);
    if (input.IsPressed(Action::PAUSE)) { state = Global::GameState::PLAYING; return; }
    if (input.IsPressed(Action::ENTER)) {
        switch (pauseCursor) {
            case 0: state = Global::GameState::PLAYING; break;
            case 1:
                Global::previousGameState = Global::GameState::PAUSED;
                state = Global::GameState::OPTIONS;
                break;
            case 2:
                bulletPool.Clear(); enemyManager.Clear(); level.Unload();
                Global::gameData = Global::GameData{};
                menuCursor = 0;
                state = Global::GameState::MENU;
                break;
        }
    }
}

void drawPaused() {
    ClearBackground({ 30, 30, 46, 255 });
    gameCamera.BeginWorldDraw();
    level.Draw(); bulletPool.Draw(); enemyManager.Draw(); player.Draw();
    gameCamera.EndWorldDraw();
    DrawHUD();
    DrawRectangle(0, 0, Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT, {0,0,0,150});
    const char* title = "Paused";
    int tfs = SF(38);
    DrawText(title, CentreText(640, title, tfs), SY(220), tfs, WHITE);
    DrawMenuList(PAUSE_ITEMS, PAUSE_COUNT, pauseCursor, 640, 310, 52, 26);
    const char* hint = "ESC to resume";
    int hfs = SF(14);
    DrawText(hint, CentreText(640, hint, hfs), SY(694), hfs, DARKGRAY);
}

// ---------------------------------------------------------------------------
// WIN
// ---------------------------------------------------------------------------
void updateWin(Global::GameState& state) {
    if (input.IsPressed(Action::ENTER)) { StartPlaying(); state = Global::GameState::PLAYING; }
    if (input.IsPressed(Action::PAUSE)) { menuCursor = 0; state = Global::GameState::MENU;    }
}

void drawWin() {
    ClearBackground({ 5, 15, 10, 255 });
    const char* title = "YOU WIN!";
    const char* sub   = "The gulch is yours, partner.";
    const char* time  = TextFormat("Time: %.1f s", Global::gameData.playtime);
    const char* cont  = "Enter / A  —  Play Again";
    const char* menu  = "ESC        —  Main Menu";
    DrawText(title, CentreText(640, title, SF(68)), SY(200), SF(68), GOLD);
    DrawText(sub,   CentreText(640, sub,   SF(24)), SY(300), SF(24), RAYWHITE);
    DrawText(time,  CentreText(640, time,  SF(20)), SY(345), SF(20), LIGHTGRAY);
    DrawText(cont,  CentreText(640, cont,  SF(20)), SY(410), SF(20), LIGHTGRAY);
    DrawText(menu,  CentreText(640, menu,  SF(20)), SY(445), SF(20), DARKGRAY);
}

// ---------------------------------------------------------------------------
// GAME OVER
// ---------------------------------------------------------------------------
void updateGameOver(Global::GameState& state) {
    if (input.IsPressed(Action::ENTER)) { StartPlaying(); state = Global::GameState::PLAYING; }
    if (input.IsPressed(Action::PAUSE)) { menuCursor = 0; state = Global::GameState::MENU;    }
}

void drawGameOver() {
    ClearBackground({ 10, 5, 5, 255 });
    const char* title = "YOU DIED";
    const char* retry = "Enter / A  —  Try Again";
    const char* menu  = "ESC        —  Main Menu";
    DrawText(title, CentreText(640, title, SF(66)), SY(240), SF(66), RED);
    DrawText(retry, CentreText(640, retry, SF(22)), SY(360), SF(22), LIGHTGRAY);
    DrawText(menu,  CentreText(640, menu,  SF(22)), SY(400), SF(22), DARKGRAY);
}
