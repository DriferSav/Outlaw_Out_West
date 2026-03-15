#include <hud.hpp>
#include <player.hpp>
#include <globals.hpp>
#include <level.hpp>
#include <raylib.h>

// All positions and sizes defined in design-space (1280x720).
// Global::DS() converts them to actual screen pixels at runtime.

void DrawHUD() {
    const float s  = Global::scale;
    const float lx = Global::letterboxX;
    const float ly = Global::letterboxY;

    // Helper: design-space rect -> screen rect
    auto R = [&](float x, float y, float w, float h) -> Rectangle {
        return { lx + x*s, ly + y*s, w*s, h*s };
    };
    // Design-space int position -> screen int
    auto X = [&](float x) -> int { return (int)(lx + x*s); };
    auto Y = [&](float y) -> int { return (int)(ly + y*s); };
    auto FS= [&](int fs)  -> int { return (int)(fs * s);   }; // font size

    // --- Health hearts (top-left) ---
    const float HW=22, HH=20, HPad=5, HX=12, HY=12;
    for (int i = 0; i < player.GetMaxHp(); i++) {
        Color c = (i < player.GetHp()) ? Color{220,50,50,255} : Color{60,20,20,180};
        DrawRectangleRec(R(HX + i*(HW+HPad), HY, HW, HH), c);
        DrawRectangleLinesEx(R(HX + i*(HW+HPad), HY, HW, HH), 1.0f, {0,0,0,100});
    }

    // --- Ammo (below hearts) ---
    const float AW=10, AH=18, APad=4, AX=12, AY=40;
    if (player.IsReloading()) {
        float pct = player.GetReloadProgress();
        DrawRectangleRec(R(AX, AY, 72*pct, 8), ORANGE);
        DrawRectangleLinesEx(R(AX, AY, 72, 8), 1.0f, {0,0,0,80});
        DrawText("RELOAD", X(AX), Y(AY+10), FS(12), ORANGE);
    } else {
        int ammo = player.GetAmmo(), max = player.GetMaxAmmo();
        for (int i = 0; i < max; i++) {
            Color c = (i < ammo) ? Color{255,220,50,255} : Color{60,60,40,180};
            DrawRectangleRec(R(AX + i*(AW+APad), AY, AW, AH), c);
        }
    }

    // --- Dash cooldown bar ---
    float dashPct = player.GetDashCooldownPct();
    if (dashPct < 1.0f) {
        DrawRectangleRec(R(12, 66, 64*dashPct, 5), SKYBLUE);
        DrawRectangleLinesEx(R(12, 66, 64, 5), 1.0f, {0,0,0,60});
    }

    // --- Inventory keys (top-right) ---
    float ix = Global::DESIGN_W - 14;
    for (const auto& item : Global::gameData.inventory) {
        ix -= 22;
        DrawRectangleRec(R(ix, 12, 18, 18), {220,170,30,255});
        DrawText("K", X(ix+4), Y(14), FS(11), WHITE);
    }

    // --- Level name (top-centre) ---
    if (level.IsLoaded()) {
        const char* name = level.GetName().c_str();
        int fs = FS(15);
        int tw = MeasureText(name, fs);
        DrawText(name,
            (int)(lx + (Global::DESIGN_W * s) * 0.5f - tw * 0.5f),
            Y(7), fs, {200,200,200,200});
    }
}
