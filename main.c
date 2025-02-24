#include <stdio.h>

#include <raylib.h>

int main() {
    SetWindowState(FLAG_WINDOW_RESIZABLE 
                   | FLAG_MSAA_4X_HINT 
                   | FLAG_VSYNC_HINT 
                   | FLAG_WINDOW_HIGHDPI);
    InitWindow(800, 600, "UI Fun");

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);
        DrawText("Hello world!", 20, 20, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();
}