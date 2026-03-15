#include <hud.hpp>
#include <player.hpp>
#include <globals.hpp>
#include <level.hpp>
#include <raylib.h>

void DrawHUD() {
    // --- Health hearts ---
    constexpr int HW = 18, HH = 16, HPad = 4, HX = 10, HY = 10;
    for (int i = 0; i < player.GetMaxHp(); i++) {
        Color c = (i < player.GetHp()) ? Color{220,50,50,255} : Color{80,30,30,180};
        DrawRectangle(HX + i*(HW+HPad), HY, HW, HH, c);
        DrawRectangleLinesEx({(float)(HX + i*(HW+HPad)),(float)HY,(float)HW,(float)HH}, 1, {0,0,0,100});
    }

    // --- Ammo display ---
    constexpr int AW=8, AH=14, APad=3, AX=10, AY=34;
    if (player.IsReloading()) {
        float pct  = player.GetReloadProgress();
        DrawRectangle(AX, AY, (int)(60*pct), 6, ORANGE);
        DrawRectangleLinesEx({(float)AX,(float)AY,60,6}, 1, {0,0,0,80});
        DrawText("RELOAD", AX, AY+8, 10, ORANGE);
    } else {
        int ammo = player.GetAmmo(), max = player.GetMaxAmmo();
        for (int i = 0; i < max; i++) {
            Color c = (i < ammo) ? Color{255,220,50,255} : Color{60,60,40,180};
            DrawRectangle(AX + i*(AW+APad), AY, AW, AH, c);
        }
    }

    // --- Dash cooldown ---
    float dashPct = player.GetDashCooldownPct();
    if (dashPct < 1.0f) {
        DrawRectangle(10, 58, (int)(50*dashPct), 4, SKYBLUE);
        DrawRectangleLinesEx({10,58,50,4}, 1, {0,0,0,60});
    }

    // --- Inventory (key icons) ---
    int ix = Global::SCREEN_WIDTH - 10;
    for (const auto& item : Global::gameData.inventory) {
        ix -= 18;
        DrawRectangle(ix, 10, 14, 14, {220,170,30,255});
        DrawText("K", ix+3, 11, 10, WHITE);
    }

    // --- Level name ---
    if (level.IsLoaded()) {
        const char* name = level.GetName().c_str();
        DrawText(name,
            Global::SCREEN_WIDTH/2 - MeasureText(name, 14)/2,
            6, 14, {200,200,200,180});
    }
}
