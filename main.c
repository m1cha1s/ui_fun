#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#define BASE_ARENA
#define BASE_IMPLEMENTATION
#include "base.h"
#define IMPL
#include "ui.h"

Arena *temp_arena = NULL;

int main() {
    temp_arena = arena_new();

    SetWindowState(FLAG_WINDOW_RESIZABLE
                   | FLAG_MSAA_4X_HINT
                   | FLAG_VSYNC_HINT
                   | FLAG_WINDOW_HIGHDPI
                   | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(800, 600, "UI Fun");
    
    ui_init();
    
    ui_state.root_node->dim.xy[0] = 0;
    ui_state.root_node->dim.xy[1] = 0;
    
    ui_state.root_node->dim.wh[0] = GetRenderWidth();
    ui_state.root_node->dim.wh[1] = GetRenderHeight();
    
    
    ui_layout(ui_state.root_node);
    
    // return 0;
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        
        ClearBackground(WHITE);
        
        ui_build_begin();
        
        ui_state.root_node->dim.wh[0] = GetRenderWidth();
        ui_state.root_node->dim.wh[1] = GetRenderHeight();
        
        UI_Node *panel = ui_panel(S("test panel"));
        ui_push_parent(panel);
        {
            UI_Node *hello_label = ui_label(S("Hello"));
            UI_Node *bye_label = ui_label(S("Byeo"));
        }
        ui_pop_parent();
        
        panel = ui_panel(S("50%"));
        panel->size[UI_Axis2_X].kind = UI_Size_Parent_Percent;
        panel->size[UI_Axis2_X].value = 0.3;
        ui_push_parent(panel);
        {
            UI_Node *a = ui_label(S("Gello"));
        }
        ui_pop_parent();
        
        ui_build_end();
        
        arena_reset(temp_arena);
        EndDrawing();
    }
    
    ui_deinit();
    CloseWindow();
    arena_free(temp_arena);
}
