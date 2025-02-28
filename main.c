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
    // InitWindow(800, 600, "UI Fun");

    // UI_State state;

    // ui_init_state(&state);
    // ui_set_state(&state);

    ui_init();

    ui_state.root_node->dim.xy[0] = 0;
    ui_state.root_node->dim.xy[1] = 0;

    ui_state.root_node->dim.wh[0] = GetRenderWidth();
    ui_state.root_node->dim.wh[1] = GetRenderHeight();

    // UI_Node *panel = ui_panel(S("test panel"));
    // ui_push_parent(panel);
    {
        UI_Node *hello_label = ui_label(S("Hello"));
        UI_Node *bye_label = ui_label(S("Bye"));
    }
    // ui_pop_parent();

    ui_layout(ui_state.root_node);

    return 0;

    while (!WindowShouldClose()) {
        BeginDrawing();

            // ui_begin_frame(); // State agnostic...

            ClearBackground(WHITE);
            DrawText("Hello world!", 20, 20, 20, BLACK);

            ui_draw(ui_state.root_node);

            arena_reset(&temp_arena);
        EndDrawing();
    }

    ui_deinit();
    CloseWindow();
}
