#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <raylib.h>

#define BASE_ARENA
#define BASE_IMPLEMENTATION
#include "base.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef STB_DS_IMPLEMENTATION
#define IMPL
#include "ui.h"

Arena *temp_arena;

char *tprintf(char *fmt, ...) {
    va_list args;
    ssize buf_size;
    char *buf;

    va_start(args, fmt);
    buf_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    buf = arena_alloc(temp_arena, buf_size+1);

    va_start(args, fmt);
    vsnprintf(buf, buf_size+1, fmt, args);
    va_end(args);

    buf[buf_size] = 0;

    return buf;
}

int main() {
    temp_arena = arena_new();

    SetWindowState(FLAG_WINDOW_RESIZABLE
                   | FLAG_MSAA_4X_HINT
                   | FLAG_VSYNC_HINT
                   | FLAG_WINDOW_HIGHDPI
                   | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(800, 600, "UI Fun");
    
    ui_state = ui_init();
    
    ui_state->root_node->dim.xy[0] = 0;
    ui_state->root_node->dim.xy[1] = 0;
    
    ui_state->root_node->dim.wh[0] = GetRenderWidth();
    ui_state->root_node->dim.wh[1] = GetRenderHeight();
    
    
    // ui_layout(ui_state->root_node);
    
    // return 0;
    
    int gello = 1;
    int list_size = 0;
    
    while (!WindowShouldClose()) {
        // if (IsKeyPressed(KEY_G)) gello = !gello;


        BeginDrawing();
        
        ClearBackground(WHITE);
        
        ui_build_begin();
        
        ui_state->root_node->dim.wh[0] = GetRenderWidth();
        ui_state->root_node->dim.wh[1] = GetRenderHeight();
        
        UI_Node *panel = ui_panel(S("test panel"), 0);
        ui_push_parent(panel);
        {
            UI_Node *hello_label = ui_label(S("Hello"), 0);
            UI_Node *bye_label = ui_label(S("Byeo"), 0);
        }
        ui_pop_parent();
        
        panel = ui_panel(S("buttons"), 0);
        ui_push_parent(panel);

        if (ui_button(S("G"), 0)) {
            printf("Pressed the button\n");
            gello = !gello;
        }
        if (ui_button(S("G on"), 0)) {
            printf("Pressed the button 1\n");
            gello = 1;
        }
        if (ui_button(S("G off"), 0)) {
            printf("Pressed the button 2\n");
            gello = 0;
        }

        ui_pop_parent();

        char *text = ui_text_input(S("Text text"), 0);
        // if (text) printf("%s\n", text);
        
        if (gello) {
            panel = ui_panel(S("50%"), 0);
            panel->size[UI_Axis2_X].kind = UI_Size_Parent_Percent;
            panel->size[UI_Axis2_X].value = 0.3;
            ui_push_parent(panel);
            {
                UI_Node *a = ui_label(S("Gello"), 0);
            }
            ui_pop_parent();
        }

        panel = ui_panel(S("list"), 0);
        panel->flags &= ~UI_LAYOUT_H;
        panel->flags |= UI_LAYOUT_V;
        ui_push_parent(panel);
        {
            panel = ui_panel(S("list ctrl"), 0);
            ui_push_parent(panel);
            {
                if (ui_button(S("list +"), 0)) {
                    ++list_size;
                }
                if (ui_button(S("list -"), 0) && list_size > 0) {
                    --list_size;
                }
                char *list_size_str = tprintf("list size: %d", list_size);
                ui_label((String){.str=list_size_str, .len=strlen(list_size_str)}, 0);
            }
            ui_pop_parent();

            if (list_size) {
                panel = ui_panel(S("list list"), 0);
                panel->flags &= ~UI_LAYOUT_H;
                panel->flags |= UI_LAYOUT_V;
                ui_push_parent(panel);
                {
                    for (int i = 0; i < list_size; ++i) {
                        char *l_name = tprintf("list item %d", i+1);
                        ui_label((String){.str=l_name, .len=strlen(l_name)}, 0);
                    }        
                }
                ui_pop_parent();
            }
        }
        ui_pop_parent();

        char *msg = tprintf("root node child count: %d", ui_state->root_node->child_count);
        ui_label((String){msg, strlen(msg)}, 0);

        ui_build_end();

        DrawFPS(700, 0);

        arena_reset(temp_arena);
        EndDrawing();
    }
    
    ui_deinit(ui_state);
    CloseWindow();
    arena_free(temp_arena);
}
