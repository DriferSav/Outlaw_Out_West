#include "nlohmann/json.hpp"
#include "raylib.h"

int main() {
    InitWindow(800, 450, "Platformer");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Works!", 300, 220, 22, WHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
