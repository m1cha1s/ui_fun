#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#define IMPL
#include "base.h"
#include "ui.h"


int main() {
    SetWindowState(FLAG_WINDOW_RESIZABLE 
                   | FLAG_MSAA_4X_HINT 
                   | FLAG_VSYNC_HINT 
                   | FLAG_WINDOW_HIGHDPI);
    InitWindow(800, 600, "UI Fun");

    UI_State state;

    UI_InitState(&state);
    UI_SetState(&state);

    while (!WindowShouldClose()) {
        DeferLoop(BeginDrawing(), EndDrawing()) {
            ArenaScope(temp_arena) {
                UI_BeginFrame(); // State agnostic...

                ClearBackground(WHITE);
                DrawText("Hello world!", 20, 20, 20, BLACK);
            }
        }
    }

    CloseWindow();
}