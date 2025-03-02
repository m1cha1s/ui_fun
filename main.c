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
    
    UI_Init();
    
    ui_state.root_node->dim.xy[0] = 0;
    ui_state.root_node->dim.xy[1] = 0;
    
    ui_state.root_node->dim.wh[0] = GetRenderWidth();
    ui_state.root_node->dim.wh[1] = GetRenderHeight();
    
    UI_Node *panel = UI_Panel(S("test panel"));
    UI_PushParent(panel);
    {
        UI_Node *hello_label = UI_Label(S("Hello"));
        UI_Node *bye_label = UI_Label(S("Byeo"));
    }
    UI_PopParent();
    
    panel = UI_Panel(S("50%"));
    panel->size[UI_Axis2_X].kind = UI_Size_Parent_Percent;
    panel->size[UI_Axis2_X].value = 0.3;
    UI_PushParent(panel);
    {
        UI_Node *a = UI_Label(S("Gello"));
    }
    UI_PopParent();
    
    UI_Layout(ui_state.root_node);
    
    // return 0;
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        
        // ui_begin_frame(); // State agnostic...
        
        ClearBackground(WHITE);
        // DrawText("Hello world!", 20, 20, 20, BLACK);
        
        ui_state.root_node->dim.wh[0] = GetRenderWidth();
        ui_state.root_node->dim.wh[1] = GetRenderHeight();
        
        
        UI_Layout(ui_state.root_node);
        UI_Draw(ui_state.root_node);
        
        ArenaReset(&temp_arena);
        EndDrawing();
    }
    
    UI_Deinit();
    CloseWindow();
}
