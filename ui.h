#ifndef _UI_H
#define _UI_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#include "base.h"
#include "ms_arena.h"

#define FONT_SIZE 20

typedef enum UI_Axis2 {
    UI_Axis2_X,
    UI_Axis2_Y,
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
    UI_LAYOUT_V        = (1ull<<4),
    UI_LAYOUT_H        = (1ull<<5),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    UI_Node *first_child;
    UI_Node *next;
    UI_Node *parent;
    
    usize child_count;
    
    // Calculated every frame;
    f32 pos_start[UI_Axis2_COUNT];
    Rect dim;
    
    UI_Size size[UI_Axis2_COUNT];
    
    UI_Flags flags;
    
    usize hash;
    String string;
    f32 pad[UI_Axis2_COUNT];
};

typedef struct UI_State {
    Arena node_arena;
    
    f32 pad[UI_Axis2_COUNT];
    
    UI_Node *root_node;
    UI_Node *parent;
} UI_State;

extern UI_State ui_state;

// Helpers

usize HashString(String str);

UI_Node *UI_MakeNode(UI_Flags flags, String id);

// Builders

void UI_Init(void);
void UI_Deinit(void);

void UI_BuildBegin(void);
void UI_BuildEnd(void);

void UI_PushParent(UI_Node *parent);
void UI_PopParent(void);

void UI_SetPadX(f32 val);
void UI_SetPadY(f32 val);

UI_Node *UI_Panel(String id);
UI_Node *UI_Label(String label);

void UI_Layout(UI_Node *node);

void UI_Draw(UI_Node *node);

#ifdef IMPL

UI_State ui_state;

usize HashString(String str) {
    usize hash = 2166136261u;
    for (usize i = 0; i < str.len; ++i) {
        hash ^= (u8)str.str[i];
        hash *= 16777619;
    }
    return hash;
}

void UI_Init(void) {
    UI_Node *node = ArenaAlloc(&ui_state.node_arena, sizeof(UI_Node));
    memset(node, 0, sizeof(*node));
    
    String id = S("_root");
    
    node->string = id;
    node->hash = HashString(id);
    node->flags = UI_LAYOUT_V;
    
    node->size[UI_Axis2_X].kind = UI_Size_Null;
    node->size[UI_Axis2_Y].kind = UI_Size_Null;
    
    node->first_child = NULL;
    node->next = NULL;
    node->child_count = 0;
    node->parent = NULL;
    
    ui_state.root_node = ui_state.parent = node;
    ui_state.pad[UI_Axis2_X] = 10;
    ui_state.pad[UI_Axis2_Y] = 10;
}

void UI_Deinit(void) {
    ArenaFree(&ui_state.node_arena);
}

UI_Node *UI_MakeNode(UI_Flags flags, String id) {
    UI_Node *node = ArenaAlloc(&ui_state.node_arena, sizeof(UI_Node));
    memset(node, 0, sizeof(*node));
    
    node->string = id;
    node->hash = HashString(id);
    node->flags = flags;
    
    node->size[UI_Axis2_X].kind = UI_Size_Null;
    node->size[UI_Axis2_Y].kind = UI_Size_Null;
    
    node->first_child = NULL;
    node->next = NULL;
    node->child_count = 0;
    
    node->pad[UI_Axis2_X] = ui_state.pad[UI_Axis2_X];
    node->pad[UI_Axis2_Y] = ui_state.pad[UI_Axis2_Y];
    
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

void UI_BuildBegin(void) {
    // ui_state.root_node =
}

void UI_BuildEnd(void) {
    
}

void UI_PushParent(UI_Node *parent) {
    ui_state.parent = parent;
}

void UI_PopParent(void) {
    ui_state.parent = ui_state.parent->parent;
}

void UI_SetPadX(f32 val) {
    ui_state.pad[UI_Axis2_X] = val;
}

void UI_SetPadY(f32 val) {
    ui_state.pad[UI_Axis2_Y] = val;
}

UI_Node *UI_Panel(String id) {
    UI_Node *panel_node = UI_MakeNode(UI_DRAW_BACKGROUND | UI_LAYOUT_H, id);
    panel_node->size[UI_Axis2_X].kind = UI_Size_Children_Sum;
    panel_node->size[UI_Axis2_Y].kind = UI_Size_Children_Sum;
    return panel_node;
}

UI_Node *UI_Label(String label) {
    UI_Node *label_node = UI_MakeNode(UI_DRAW_TEXT | UI_DRAW_BACKGROUND, label);
    label_node->size[UI_Axis2_X].kind = UI_Size_Text_Content;
    label_node->size[UI_Axis2_Y].kind = UI_Size_Text_Content;
    
    UI_Node *child = ui_state.parent->first_child;
    while (child) child = child->next;
    
    child = label_node;
    ++ui_state.parent->child_count;
    
    return label_node;
}

// FIXME: change from iterating over the children to building self with parent as ref, maybe.
void UI_Layout(UI_Node *node)
{
    Vector2 text_size;
    UI_Node *child;
    if (!node) return;
    
    node->pos_start[UI_Axis2_X] = 0;
    node->pos_start[UI_Axis2_Y] = 0;
    
    UI_Node *parent = node->parent;
    
    if (!parent) {
        UI_Layout(node->first_child);
        goto exit; // This should only be true for the root node.
    }
    
    node->pos_start[UI_Axis2_X] = parent->pos_start[UI_Axis2_X]+node->pad[UI_Axis2_X];
    node->pos_start[UI_Axis2_Y] = parent->pos_start[UI_Axis2_Y]+node->pad[UI_Axis2_Y];
    
    node->dim.xy[UI_Axis2_X] = parent->pos_start[UI_Axis2_X];
    node->dim.xy[UI_Axis2_Y] = parent->pos_start[UI_Axis2_Y];
    
    UI_Layout(node->first_child);
    
    for (int ax = UI_Axis2_X; ax < UI_Axis2_COUNT; ++ax)
    {
        switch (node->size[ax].kind)
        {
            case UI_Size_Null: break;
            case UI_Size_Parent_Percent:
            node->dim.wh[ax] = parent->dim.wh[ax]*node->size[ax].value;
            break;
            case UI_Size_Pixels:
            node->dim.wh[ax] = node->size[ax].value;
            break;
            case UI_Size_Children_Sum:
            node->dim.wh[ax] = node->pos_start[ax]-node->dim.xy[ax] + node->pad[ax];
            
            // UI_Layout(node->first_child);
            if (node->dim.wh[ax] == 2*node->pad[ax]) {
                child = node->first_child;
                while (child) {
                    node->dim.wh[ax] = Max(child->dim.wh[ax] + 2*node->pad[ax], node->dim.wh[ax]);
                    child = child->next;
                }
            }
            break;
            case UI_Size_Text_Content:
            text_size = MeasureTextEx(
                                      GetFontDefault(),
                                      (const char*)node->string.str,
                                      FONT_SIZE,
                                      FONT_SIZE/10
                                      );
            f32 xy[UI_Axis2_COUNT] = {text_size.x, text_size.y};
            node->dim.wh[ax] = xy[ax]+2*node->pad[ax];
            break;
            default:
            break;
        }
    }
    
    if (parent->flags & UI_LAYOUT_H) {
        parent->pos_start[UI_Axis2_X] += node->dim.wh[UI_Axis2_X];
    }
    if (parent->flags & UI_LAYOUT_V) {
        parent->pos_start[UI_Axis2_Y] += node->dim.wh[UI_Axis2_Y];
    }
    
    exit:
    UI_Layout(node->next);
}

void UI_Draw(UI_Node *node) {
    if (!node) return;
    Rectangle r = {
        .x = node->dim.xy[UI_Axis2_X],
        .y = node->dim.xy[UI_Axis2_Y],
        .width = node->dim.wh[UI_Axis2_X],
        .height = node->dim.wh[UI_Axis2_Y],
    };
    const Color colors[] = {
        YELLOW,
        GOLD,
        ORANGE,
        PINK,
        RED,
        MAROON,
        GREEN,
        LIME,
        SKYBLUE,
        BLUE,
        PURPLE,
        VIOLET,
    };
    // printf("%p %p %f %f %f %f\n", node, node->parent, r.x, r.y, r.width, r.height);
    
    
    if (node->flags & UI_DRAW_BACKGROUND) DrawRectangleRec(r, colors[node->hash%ArrayLen(colors)]);
    if (node->flags & UI_DRAW_TEXT) DrawText(
                                             (const char *)node->string.str,
                                             node->dim.xy[0]+node->pad[0],
                                             node->dim.xy[1]+node->pad[1],
                                             FONT_SIZE,
                                             BLACK
                                             );
    
    UI_Draw(node->first_child);
    UI_Draw(node->next);
}

#endif

#endif
