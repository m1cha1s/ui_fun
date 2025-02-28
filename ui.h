#ifndef _UI_H
#define _UI_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#include "types.h"
#include "ms_arena.h"

#define FONT_SIZE 20

typedef enum UI_Axis2 {
    UI_Axis2_x,
    UI_Axis2_y,
    UI_Axis2_COUNT,
} UI_Axis2;

typedef struct Rect {
    f32 xy[2], wh[2];
} Rect;

typedef enum UI_Size_Kind {
    UI_Size_Null,
    UI_Size_Parent_Percent,
    UI_Size_Text_Content,
    UI_Size_Pixels,
    UI_Size_Children_Sum,
} UI_Size_Kind;

typedef struct UI_Size {
    UI_Size_Kind kind;
    f32 value;
    f32 strictness; // TODO: Use it...
} UI_Size;

typedef u32 UI_Flags;
enum {
    UI_DRAW_TEXT       = (1ull<<0),
    UI_DRAW_BACKGROUND = (1ull<<1),
    UI_DRAW_FRAME      = (1ull<<2),
    UI_CLICKABLE       = (1ull<<3),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    UI_Node *first_child;
    UI_Node *next;
    UI_Node *parent;

    usize child_count;

    UI_Size size[UI_Axis2_COUNT];

    UI_Flags flags;

    usize hash;
    String string;

    Rect dim;
};

typedef struct UI_State {
    Arena node_arena;

    UI_Node *root_node;
    UI_Node *parent;
} UI_State;

extern UI_State ui_state;

// Helpers

usize hash_string(u8 *str, usize len);
#define hash_String(s) hash_string((s).str, (s).len)

UI_Node *ui_make_node(UI_Flags flags, String id);

// Builders

void ui_init(void);

void ui_build_begin(void);
void ui_build_end(void);

void ui_push_parent(UI_Node *parent);
void ui_pop_parent(void);

UI_Node *ui_label(String label);

void ui_layout(UI_Node *node);

void ui_draw(UI_Node *node);

#ifdef IMPL

UI_State ui_state;

usize hash_string(u8 *str, usize len) {
    usize hash = 2166136261u;
    for (usize i = 0; i < len; ++i) {
        hash ^= (u8)str[i];
        hash *= 16777619;
    }
    return hash;
}

void ui_init(void) {
    UI_Node *node = arena_alloc(&ui_state.node_arena, sizeof(UI_Node));

    String id = S("_root");

    node->string = id;
    node->hash = hash_String(id);
    node->flags = 0;

    node->size[UI_Axis2_x].kind = UI_Size_Null;
    node->size[UI_Axis2_y].kind = UI_Size_Null;

    node->first_child = NULL;
    node->next = NULL;
    node->child_count = 0;

    ui_state.root_node = ui_state.parent = node;
}

UI_Node *ui_make_node(UI_Flags flags, String id) {
    UI_Node *node = arena_alloc(&ui_state.node_arena, sizeof(UI_Node));

    node->string = id;
    node->hash = hash_String(id);
    node->flags = flags;

    node->size[UI_Axis2_x].kind = UI_Size_Null;
    node->size[UI_Axis2_y].kind = UI_Size_Null;

    node->first_child = NULL;
    node->next = NULL;
    node->child_count = 0;

    node->parent = ui_state.parent;

    UI_Node *pchild = ui_state.parent->first_child;
    if (pchild) {
        while (pchild->next) pchild = pchild->next;
        pchild->next = node;
    } else {
        ui_state.parent->first_child = node;
    }


    return node;
}

void ui_build_begin(void) {
    // ui_state.root_node = 
}

void ui_build_end(void) {

}

void ui_push_parent(UI_Node *parent) {
    ui_state.parent = parent;
}

void ui_pop_parent(void) {
    ui_state.parent = ui_state.parent->parent;
}

UI_Node *ui_label(String label) {
    UI_Node *label_node = ui_make_node(UI_DRAW_TEXT | UI_DRAW_BACKGROUND, label);
    label_node->size[UI_Axis2_x].kind = UI_Size_Text_Content;
    label_node->size[UI_Axis2_y].kind = UI_Size_Text_Content;

    UI_Node *child = ui_state.parent->first_child;
    while (child) child = child->next;

    child = label_node;
    ++ui_state.parent->child_count;

    return label_node;
}

// FIXME: change from iterating over the children to building self with parent as ref, maybe.
void ui_layout(UI_Node *root)
{
    Vector2 text_size;

    f32 node_start[UI_Axis2_COUNT];
    node_start[UI_Axis2_x] = root->dim.xy[UI_Axis2_x];
    node_start[UI_Axis2_y] = root->dim.xy[UI_Axis2_y];

    UI_Node *child = root->first_child;
    for (usize i = 0; i < root->child_count; ++i) 
    {
        for (int ax = UI_Axis2_x; ax < UI_Axis2_COUNT; ++ax)
        {
            // 1st: Sizing
            if (!child) break;
            switch (child->size[ax].kind)
            {
            case UI_Size_Null: break;
            case UI_Size_Parent_Percent:
                child->dim.wh[ax] = root->dim.wh[ax]*child->size[ax].value;
                break;
            case UI_Size_Pixels:
                child->dim.wh[ax] = child->size[ax].value;
                break;
            case UI_Size_Children_Sum:
                child->dim.wh[ax] = 0;
                ui_layout(child);
                UI_Node *child2 = child->first_child;
                for (usize j = 0; j < child->child_count; ++j) {
                    child->dim.wh[ax] += child2->dim.wh[ax];
                    child2 = child2->next;
                }
                break;
            case UI_Size_Text_Content:
                text_size = MeasureTextEx(GetFontDefault(), (const char*)child->string.str, FONT_SIZE, FONT_SIZE/10);
                f32 xy[UI_Axis2_COUNT] = {text_size.x, text_size.y};
                // f32 xy[UI_Axis2_COUNT] = {0, 0};
                printf("Foo\n");
                child->dim.wh[ax] = xy[ax];
                break;
            default:
                break;
            }
            
            child->dim.xy[ax] = node_start[ax];
            node_start[ax] += child->dim.wh[ax];
        }
        if (!child) break;
        child = child->next;
    }
}

void ui_draw(UI_Node *node) {
    if (!node) return;
    Rectangle r = {
        .x = node->dim.xy[UI_Axis2_x],
        .y = node->dim.xy[UI_Axis2_y],
        .width = node->dim.wh[UI_Axis2_x],
        .height = node->dim.wh[UI_Axis2_y],
    };
    const Color colors[3] = {RED, BLUE, GREEN};
    // printf("%f %f %f %f\n", r.x, r.y, r.width, r.height);
    
    if (node->flags & UI_DRAW_BACKGROUND) DrawRectangleRec(r, colors[node->hash%3]);
    if (node->flags & UI_DRAW_TEXT) DrawText((const char *)node->string.str, node->dim.xy[0], node->dim.xy[1], FONT_SIZE, BLACK);


    ui_draw(node->next);
    ui_draw(node->first_child);
}

#endif

#endif
