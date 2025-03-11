#ifndef _UI_H
#define _UI_H

/*

Frame sequence:
- ui_begin_build
  - [x] Reset
- ui_make_node
  - [x] Construction & Event dispatch based on the last frame
- ui_end_build
  - [x] Prune/Remove unused cache slots
  - [x] Layout
  - [x] Handle events for this frame
  - [x] Draw

*/

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <raylib.h>

#define BASE_ARENA
#include "base.h"
#include "stb_ds.h"

#define FONT_SIZE 20

typedef struct Vec2 {
    f32 x, y;
} Vec2;

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

typedef enum UI_Key {
    UI_MOUSE_LEFT,
    UI_MOUSE_RIGHT,
} UI_Key;

typedef enum UI_Event_Kind {
    UI_EVENT_NULL,
    UI_EVENT_PRESS,
    UI_EVENT_RELEASE,
    UI_EVENT_MOUSE_MOVE,
    UI_EVENT_SCROLL,
} UI_Event_Kind;

typedef struct UI_Event {
    UI_Event_Kind kind;
    UI_Key key;
    Vec2 pos;
    Vec2 delta;
} UI_Event;

typedef u32 UI_Flags;
enum {
    UI_DRAW_TEXT       = (1ull<<0),
    UI_DRAW_BACKGROUND = (1ull<<1),
    UI_DRAW_BORDER     = (1ull<<2),
    UI_CLICKABLE       = (1ull<<3),
    UI_LAYOUT_V        = (1ull<<4),
    UI_LAYOUT_H        = (1ull<<5),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    // Builder filled
    UI_Node *parent;
    UI_Node *first_child;
    UI_Node *next;
    usize child_count;
    
    f32 pad[UI_Axis2_COUNT];
    UI_Flags flags;
    UI_Size size[UI_Axis2_COUNT];

    usize hash;
    String string;

    // Calculated every frame;
    f32 pos_start[UI_Axis2_COUNT];
    Rect dim;
};

typedef struct UI_Node_Data {
    usize frame_number;

    String key;

    // Last frame event info also lands here, used by builders to report events to the caller.
    UI_Event event;
} UI_Node_Data;

typedef struct UI_Node_Data_KV {
    usize key;
    UI_Node_Data value;
} UI_Node_Data_KV;

typedef struct UI_State {
    Arena *arena;

    usize frame_number;

    Arena *build_arena;

    f32 pad[UI_Axis2_COUNT];
    
    UI_Node *root_node;
    UI_Node *parent;

    UI_Node_Data_KV *node_data;

    UI_Event *event_buffer;
    usize focused;
} UI_State;

extern UI_State ui_state;

// Helpers

usize hash_string(String str);

UI_Node *ui_make_node(UI_Flags flags, String id);

// Builders

void ui_init(void);
void ui_deinit(void);

void ui_build_begin(void);
void ui_build_end(void);

void ui_prune(void);
void ui_collect_events(void);
void ui_dispatch_events(void);

void ui_push_parent(UI_Node *parent);
void ui_pop_parent(void);

void ui_set_pad_x(f32 val);
void ui_set_pad_y(f32 val);

UI_Node *ui_panel(String id);
UI_Node *ui_label(String label);
int ui_button(String label);

void ui_layout(UI_Node *node);

void ui_draw(UI_Node *node);

#ifdef IMPL

UI_State ui_state;

usize hash_string(String str) {
    usize hash = 2166136261u;
    for (usize i = 0; i < str.len; ++i) {
        hash ^= (u8)str.str[i];
        hash *= 16777619;
    }
    return hash;
}

void ui_init(void) {
    memory_set(&ui_state, 0, sizeof(UI_State));
    
    ui_state.arena = arena_new();
    
    ui_state.build_arena = arena_new();
    
    UI_Node *node = arena_alloc(ui_state.arena, sizeof(UI_Node));
    memory_set(node, 0, sizeof(*node));
    
    String id = S("_root");
    
    node->string = id;
    node->hash = hash_string(id);
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

    ui_state.focused = node->hash;
}

void ui_deinit(void) {
    arena_free(ui_state.build_arena);
    arena_free(ui_state.arena);
    arrfree(ui_state.event_buffer);
}

UI_Node *ui_make_node(UI_Flags flags, String id) {
    UI_Node *node = arena_alloc(ui_state.build_arena, sizeof(UI_Node));
    memory_set(node, 0, sizeof(*node));
    
    node->string = id;
    node->hash = hash_string(id);
    node->flags = flags;

    ssize idx = hmgeti(ui_state.node_data, node->hash);
    if (idx < 0) // New node
        hmput(ui_state.node_data, node->hash, ((UI_Node_Data){.key=id, .frame_number=ui_state.frame_number}));
    else
        ui_state.node_data[idx].value.frame_number = ui_state.frame_number;
    
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

    ui_state.parent->child_count += 1;

    return node;
}

void ui_build_begin(void) {
    arena_reset(ui_state.build_arena);

    ui_state.root_node->first_child = NULL;
    ui_state.root_node->parent = NULL;
    ui_state.root_node->next = NULL;
    ui_state.root_node->child_count = 0;

    ui_state.frame_number += 1;
}

void ui_build_end(void) {
    ui_prune();
    ui_layout(ui_state.root_node);
    ui_collect_events();
    ui_dispatch_events();
    ui_draw(ui_state.root_node);
}

void ui_prune(void) {
    for (usize i = 0; i < hmlen(ui_state.node_data); ++i) {
        if (ui_state.node_data[i].value.frame_number != ui_state.frame_number && 
            ui_state.node_data[i].key != ui_state.root_node->hash) {
            UI_Node_Data_KV node_data_kv = ui_state.node_data[i];
            printf("prune idx: %llu key: %.*s(%llu)\n", i, node_data_kv.value.key.len, node_data_kv.value.key.str, node_data_kv.key);
            hmdel(ui_state.node_data, ui_state.node_data[i].key);
        } else {
            ui_state.node_data[i].value.event = (UI_Event){0};
        }
    }
}

static int point_in_rect(Vec2 p, Rect r) {
    return (p.x > r.xy[0]) && (p.x < r.xy[0]+r.wh[0]) && (p.y > r.xy[1]) && (p.y < r.xy[1]+r.wh[1]);
}

void ui_collect_events(void) {
    Vector2 mouse_pos = GetMousePosition();
    Vec2 m_pos = (Vec2){mouse_pos.x, mouse_pos.y};

    Vector2 mouse_delta = GetMouseDelta();
    int mouse_moved = mouse_delta.x || mouse_delta.y;

    if (mouse_moved) arrpush(ui_state.event_buffer, ((UI_Event){.kind=UI_EVENT_MOUSE_MOVE, .pos=m_pos}));
    if (IsMouseButtonPressed(0))  arrpush(ui_state.event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_MOUSE_LEFT, .pos=m_pos}));
    if (IsMouseButtonPressed(1))  arrpush(ui_state.event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_MOUSE_RIGHT, .pos=m_pos}));
    if (IsMouseButtonReleased(0)) arrpush(ui_state.event_buffer, ((UI_Event){.kind=UI_EVENT_RELEASE, .key=UI_MOUSE_LEFT, .pos=m_pos}));
    if (IsMouseButtonReleased(1)) arrpush(ui_state.event_buffer, ((UI_Event){.kind=UI_EVENT_RELEASE, .key=UI_MOUSE_RIGHT, .pos=m_pos}));
}

void ui_dispatch_events(void) {
    for (usize i = 0; i < arrlen(ui_state.event_buffer); ++i) {
        UI_Node *current = ui_state.root_node;
        UI_Event ev = ui_state.event_buffer[i];
        UI_Node_Data_KV *data = NULL;

        switch (ev.kind) {
            case UI_EVENT_MOUSE_MOVE:
                 do {
                    if (point_in_rect(ev.pos, current->dim)) {
                        if (current->first_child) {
                            current = current->first_child;
                            continue;
                        } else {
                            ui_state.focused = current->hash;
                            break;
                        }
                    } else {
                        if (!current->next && current->parent)
                            ui_state.focused = current->parent->hash;
                        current = current->next;
                    }
                } while (current); 
                break;
            case UI_EVENT_PRESS:
            case UI_EVENT_RELEASE:
                data = hmgetp(ui_state.node_data, ui_state.focused);
                data->value.event = ev;
                break;
        }
    }

    arrsetlen(ui_state.event_buffer, 0);
}

void ui_push_parent(UI_Node *parent) {
    ui_state.parent = parent;
}

void ui_pop_parent(void) {
    ui_state.parent = ui_state.parent->parent;
}

void ui_set_pad_x(f32 val) {
    ui_state.pad[UI_Axis2_X] = val;
}

void ui_set_pad_y(f32 val) {
    ui_state.pad[UI_Axis2_Y] = val;
}

UI_Node *ui_panel(String id) {
    UI_Node *panel_node = ui_make_node(UI_DRAW_BACKGROUND | UI_LAYOUT_H, id);
    panel_node->size[UI_Axis2_X].kind = UI_Size_Children_Sum;
    panel_node->size[UI_Axis2_Y].kind = UI_Size_Children_Sum;
    return panel_node;
}

UI_Node *ui_label(String label) {
    UI_Node *label_node = ui_make_node(UI_DRAW_TEXT, label);
    label_node->size[UI_Axis2_X].kind = UI_Size_Text_Content;
    label_node->size[UI_Axis2_Y].kind = UI_Size_Text_Content;
    
    return label_node;
}

int ui_button(String label) {
    UI_Node *button_node = ui_make_node(UI_DRAW_TEXT | UI_DRAW_BACKGROUND | UI_DRAW_BORDER, label);
    
    button_node->size[UI_Axis2_X].kind = UI_Size_Text_Content;
    button_node->size[UI_Axis2_Y].kind = UI_Size_Text_Content;
    
    ssize idx = hmgeti(ui_state.node_data, button_node->hash); // idx should never be -1
    assert(idx >= 0);

    UI_Event ev = ui_state.node_data[idx].value.event;
    
    return ev.kind == UI_EVENT_PRESS && ev.key == UI_MOUSE_LEFT;
}

// FIXME: change from iterating over the children to building self with parent as ref, maybe.
void ui_layout(UI_Node *node)
{
    Vector2 text_size;
    UI_Node *child;
    if (!node) return;
    
    node->pos_start[UI_Axis2_X] = 0;
    node->pos_start[UI_Axis2_Y] = 0;
    
    UI_Node *parent = node->parent;
    
    if (!parent) {
        ui_layout(node->first_child);
        goto exit; // This should only be true for the root node.
    }
    
    node->pos_start[UI_Axis2_X] = parent->pos_start[UI_Axis2_X]+node->pad[UI_Axis2_X];
    node->pos_start[UI_Axis2_Y] = parent->pos_start[UI_Axis2_Y]+node->pad[UI_Axis2_Y];
    
    node->dim.xy[UI_Axis2_X] = parent->pos_start[UI_Axis2_X];
    node->dim.xy[UI_Axis2_Y] = parent->pos_start[UI_Axis2_Y];
    
    ui_layout(node->first_child);
    
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
            
            // ui_layout(node->first_child);
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
    ui_layout(node->next);
}

void ui_draw(UI_Node *node) {
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

    int i = 0;

    if (node->hash == ui_state.focused) i = 1;
    
    if (node->flags & UI_DRAW_BACKGROUND)
        DrawRectangleRec(r, colors[(node->hash+i)%ArrayLen(colors)]);
    if (node->flags & UI_DRAW_BORDER)
        DrawRectangleLinesEx(r, 5, colors[(node->hash+i+1)%ArrayLen(colors)]);
    if (node->flags & UI_DRAW_TEXT)
        DrawText((const char *)node->string.str,
                 node->dim.xy[0]+node->pad[0],
                 node->dim.xy[1]+node->pad[1],
                 FONT_SIZE,
                 i ? WHITE : BLACK);
    
    ui_draw(node->first_child);
    ui_draw(node->next);
}

#endif

#endif
