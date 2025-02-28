#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#define MS_ARENA_IMPLEMENTATION
#include "ms_arena.h"
#define IMPL
#include "ui.h"

Arena temp_arena = {0};

int main() {
    SetWindowState(FLAG_WINDOW_RESIZABLE
                   | FLAG_MSAA_4X_HINT
                   | FLAG_VSYNC_HINT
                   | FLAG_WINDOW_HIGHDPI
                   | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(800, 600, "UI Fun");

    // UI_State state;

    // ui_init_state(&state);
    // ui_set_state(&state);

    ui_init();

    UI_Node *hello_label = ui_label(S("Hello"));
    UI_Node *bye_label = ui_label(S("Bye"));

    ui_layout(ui_state.root_node);

    while (!WindowShouldClose()) {
        BeginDrawing();

            // ui_begin_frame(); // State agnostic...

            ClearBackground(WHITE);
            DrawText("Hello world!", 20, 20, 20, BLACK);

            ui_draw(ui_state.root_node);

            arena_reset(&temp_arena);
        EndDrawing();
    }

    CloseWindow();
}