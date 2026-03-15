// debug.cpp — only compiled when GAME_DEBUG is defined (Debug build type).
// Contains the full debug overlay: collider wireframes, player state, FPS.

#ifdef GAME_DEBUG

#include <raylib.h>
#include <globals.hpp>
#include <player.hpp>
#include <level.hpp>
#include <camera.hpp>
#include <enemy.hpp>
#include <bullet.hpp>

// ---- State toggled with F3 ----
static bool showColliders  = true;
static bool showPlayerInfo = true;
static bool showEnemyInfo  = false;

void DebugUpdate() {
    if (IsKeyPressed(KEY_F3)) showColliders  = !showColliders;
    if (IsKeyPressed(KEY_F4)) showPlayerInfo = !showPlayerInfo;
    if (IsKeyPressed(KEY_F5)) showEnemyInfo  = !showEnemyInfo;
}

// ---------------------------------------------------------------------------
// World-space debug: collider outlines, camera target cross
// Call inside BeginMode2D / EndMode2D
// ---------------------------------------------------------------------------
void DebugDrawWorld() {
    if (!level.IsLoaded()) return;

    if (showColliders) {
        // Solid colliders — red outline
        for (const Rectangle& r : level.GetSolidColliders())
            DrawRectangleLinesEx(r, 1.5f, { 255, 60, 60, 160 });

        // One-sided colliders — cyan outline
        for (const Rectangle& r : level.GetOneSidedColliders())
            DrawRectangleLinesEx(r, 1.5f, { 60, 220, 255, 160 });

        // Player bounds — green outline
        Vector2 pos = player.GetPosition();
        DrawRectangleLinesEx({ pos.x, pos.y, 32.0f, 48.0f }, 1.5f, { 60, 255, 60, 220 });
    }

    // Camera target cross
    const Vector2 ct = gameCamera.Raw().target;
    DrawLine((int)ct.x - 12, (int)ct.y, (int)ct.x + 12, (int)ct.y, YELLOW);
    DrawLine((int)ct.x, (int)ct.y - 12, (int)ct.x, (int)ct.y + 12, YELLOW);
    DrawCircleLines((int)ct.x, (int)ct.y, 4, YELLOW);
}

// ---------------------------------------------------------------------------
// Screen-space debug: stats panel in bottom-left corner
// Call after EndMode2D
// ---------------------------------------------------------------------------
void DebugDrawScreen() {
    const float s  = Global::scale;
    const float lx = Global::letterboxX;
    const float ly = Global::letterboxY;
    const int   fs = (int)(12 * s);
    const int   lh = (int)(15 * s); // line height

    int x = (int)lx + 4;
    int y = Global::SCREEN_HEIGHT - (int)ly - lh;

    // Helper: draw one line and step up
    auto line = [&](const char* txt, Color c = GREEN) {
        DrawText(txt, x, y, fs, c);
        y -= lh;
    };

    // Toggles hint
    line("F3=colliders  F4=player  F5=enemies", DARKGRAY);

    if (showPlayerInfo) {
        Vector2 pos = player.GetPosition();
        static const char* movNames[] = {"IDLE","WALK","RUN","JUMP","DASH","WALL"};
        static const char* envNames[] = {"GROUND","RISING","FALLING"};
        static const char* actNames[] = {"IDLE","ATTACK","MELEE"};

        int mov = (int)player.GetMovementState();
        int env = (int)player.GetEnvironmentState();
        int act = (int)player.GetInteractionState();

        line(TextFormat("act=%s",  actNames[act < 3 ? act : 0]), LIGHTGRAY);
        line(TextFormat("env=%s",  envNames[env < 3 ? env : 0]), LIGHTGRAY);
        line(TextFormat("mov=%s",  movNames[mov < 6 ? mov : 0]), LIGHTGRAY);
        line(TextFormat("ammo=%d/%d  reload=%.1f",
            player.GetAmmo(), player.GetMaxAmmo(),
            player.IsReloading() ? player.GetReloadProgress() : 0.f), LIGHTGRAY);
        line(TextFormat("hp=%d/%d", player.GetHp(), player.GetMaxHp()), LIGHTGRAY);
        line(TextFormat("pos=(%.0f, %.0f)  vx=%.0f", pos.x, pos.y, player.GetVelocityX()), LIGHTGRAY);
    }

    if (showEnemyInfo) {
        line(TextFormat("bullets=%d", bulletPool.ActiveCount()), ORANGE);
    }

    // Always-on: FPS and scale
    line(TextFormat("FPS=%d  scale=%.2f  %dx%d",
        GetFPS(), Global::scale, Global::SCREEN_WIDTH, Global::SCREEN_HEIGHT), LIME);
}

#endif // GAME_DEBUG
