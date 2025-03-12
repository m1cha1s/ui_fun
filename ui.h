#ifndef _UI_H
#define _UI_H

/*

TODO:
- [ ] Sensible color scheme
- [ ] More actions in the text editor
- [ ] Some example programs
  - [ ] Text editor
  - [ ] Music player

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
    UI_Size_Ed_Text_Content,
    UI_Size_Pixels,
    UI_Size_Children_Sum,
} UI_Size_Kind;

typedef struct UI_Size {
    UI_Size_Kind kind;
    f32 value;
    f32 strictness; // TODO: Use it...
} UI_Size;

typedef enum UI_Key {
    UI_MOUSE_LEFT = 256, // After ASCII
    UI_MOUSE_RIGHT,
    
    UI_LEFT,
    UI_RIGHT,
    UI_UP,
    UI_DOWN,
    
    UI_BACKSPACE,
    UI_DELETE,
} UI_Key;

typedef u32 UI_Mod;
enum {
    UI_MOD_L_SHIFT = (1ull<<0),
    UI_MOD_R_SHIFT = (1ull<<1),
};

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
    UI_Mod mod;
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
    UI_TEXT_NO_ED      = (1ull<<6),
    UI_DRAW_ED_TEXT    = (1ull<<7),
    UI_DRAW_CURSOR     = (1ull<<8),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    // Builder filled
    UI_Node *parent;
    UI_Node *first_child, *last_child;
    UI_Node *next, *prev;
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
    UI_Node *node;

    ssize cursor, mark;
    u8 *ed_string;

    // Last frame event info also lands here, used by builders to report events to the caller.
    UI_Event event;
} UI_Node_Data;

typedef struct UI_Node_Data_KV {
    usize key;
    UI_Node_Data value;
} UI_Node_Data_KV;

typedef enum UI_Mode {
    UI_MODE_NORMAL, // Standard navigation and mouse clicking.
    UI_MODE_EDIT, // Mainly text editing a text field.
} UI_Mode;

typedef struct UI_State {
    Arena *arena;

    usize frame_number;

    Arena *temp_arena;
    Arena *build_arena;

    f32 pad[UI_Axis2_COUNT];
    
    UI_Node *root_node;
    UI_Node *parent;

    UI_Mode mode;

    UI_Node_Data_KV *node_data;

    UI_Event *event_buffer;

    usize hovering;
    usize focused;

    // Color schemes
    Color text_color[3];
    Color background_color[3];
    Color border_color[3];
} UI_State;

extern UI_State *ui_state;

// Helpers

usize hash_string(String str);

UI_Node *ui_make_node(UI_Flags flags, String id);

// Builders

UI_State *ui_init(void);
void ui_deinit(UI_State *sp);

void ui_build_begin(void);
void ui_build_end(void);

void ui_prune(void);
void ui_collect_events(void);
void ui_dispatch_events(void);

void ui_push_parent(UI_Node *parent);
void ui_pop_parent(void);

UI_Node *ui_panel(String id, UI_Flags flags);
UI_Node *ui_label(String label, UI_Flags flags);
int ui_button(String label, UI_Flags flags);
char *ui_text_input(String label, UI_Flags flags);

void ui_layout(UI_Node *node);

void ui_draw(UI_Node *node);

#ifdef IMPL

UI_State *ui_state;

usize hash_string(String str) {
    usize hash = 2166136261u;
    for (usize i = 0; i < str.len; ++i) {
        hash ^= (u8)str.str[i];
        hash *= 16777619;
    }
    return hash;
}

UI_State *ui_init(void) {
    UI_State *sp = malloc(sizeof(UI_State));

    memory_set(sp, 0, sizeof(UI_State));
    
    sp->arena = arena_new();

    sp->temp_arena = arena_new();    
    sp->build_arena = arena_new();

    sp->mode = UI_MODE_NORMAL;
    
    UI_Node *node = arena_alloc(sp->arena, sizeof(UI_Node));
    memory_set(node, 0, sizeof(*node));
    
    String id = S("_root");
    
    node->string = id;
    node->hash = hash_string(id);
    node->flags = UI_LAYOUT_V;

    node->size[UI_Axis2_X].kind = UI_Size_Null;
    node->size[UI_Axis2_Y].kind = UI_Size_Null;
    
    node->first_child = NULL;
    node->last_child = NULL;
    node->next = NULL;
    node->prev = NULL;
    node->child_count = 0;
    node->parent = NULL;
    
    sp->root_node = sp->parent = node;
    sp->pad[UI_Axis2_X] = 10;
    sp->pad[UI_Axis2_Y] = 10;

    sp->focused = node->hash;
    sp->hovering = node->hash;

    sp->background_color[0] = (Color){120, 120, 120, 255};
    sp->background_color[1] = (Color){120, 120, 120, 255};
    sp->background_color[2] = (Color){120, 120, 120, 255};

    sp->border_color[0] = (Color){10, 10, 10, 255};
    sp->border_color[1] = (Color){10, 10, 10, 255};
    sp->border_color[2] = (Color){10, 10, 10, 255};

    sp->text_color[0] = (Color){255, 255, 255, 255};
    sp->text_color[1] = (Color){255, 255, 255, 255};
    sp->text_color[2] = (Color){255, 255, 255, 255};

    return sp;
}

void ui_deinit(UI_State *sp) {
    arena_free(sp->build_arena);
    arena_free(sp->arena);
    arrfree(sp->event_buffer);
    free(sp);
}

UI_Node *ui_make_node(UI_Flags flags, String id) {
    UI_Node *node = arena_alloc(ui_state->build_arena, sizeof(UI_Node));
    memory_set(node, 0, sizeof(*node));
    
    node->string = id;
    node->hash = hash_string(id);
    node->flags = flags;

    ssize idx = hmgeti(ui_state->node_data, node->hash);
    if (idx < 0) // New node
        hmput(ui_state->node_data, node->hash, ((UI_Node_Data){.key=id, .frame_number=ui_state->frame_number, .node=node}));
    else {
        ui_state->node_data[idx].value.frame_number = ui_state->frame_number;
        ui_state->node_data[idx].value.node = node;
    }
    
    node->size[UI_Axis2_X].kind = UI_Size_Null;
    node->size[UI_Axis2_Y].kind = UI_Size_Null;
    
    node->first_child = NULL;
    node->last_child = NULL;
    node->next = NULL;
    node->prev = NULL;
    node->child_count = 0;
    
    node->pad[UI_Axis2_X] = ui_state->pad[UI_Axis2_X];
    node->pad[UI_Axis2_Y] = ui_state->pad[UI_Axis2_Y];
    
    node->parent = ui_state->parent;
    
    UI_Node *pchild = ui_state->parent->last_child;
    if (pchild) {
        pchild->next = node;
        node->prev = pchild;
        ui_state->parent->last_child = node;
    } else {
        ui_state->parent->first_child = node;
        ui_state->parent->last_child = node;
    }

    ui_state->parent->child_count += 1;

    return node;
}

void ui_build_begin(void) {
    arena_reset(ui_state->build_arena);

    ui_state->root_node->first_child = NULL;
    ui_state->root_node->last_child = NULL;
    ui_state->root_node->parent = NULL;
    ui_state->root_node->next = NULL;
    ui_state->root_node->prev = NULL;
    ui_state->root_node->child_count = 0;

    ssize idx = hmgeti(ui_state->node_data, ui_state->root_node->hash);
    if (idx < 0) // New node
        hmput(ui_state->node_data, ui_state->root_node->hash, ((UI_Node_Data){.key=ui_state->root_node->string, .frame_number=ui_state->frame_number, .node=ui_state->root_node}));
    else {
        ui_state->node_data[idx].value.frame_number = ui_state->frame_number;
        ui_state->node_data[idx].value.node = ui_state->root_node;
    }

    ui_state->frame_number += 1;
}

void ui_build_end(void) {
    ui_prune();
    ui_layout(ui_state->root_node);
    ui_collect_events();
    ui_dispatch_events();
    ui_draw(ui_state->root_node);
    
    arena_reset(ui_state->temp_arena);
}

void ui_prune(void) {
    for (usize i = 0; i < hmlen(ui_state->node_data); ++i) {
        if (ui_state->node_data[i].value.frame_number != ui_state->frame_number && 
            ui_state->node_data[i].key != ui_state->root_node->hash) {
            UI_Node_Data_KV node_data_kv = ui_state->node_data[i];
            printf("prune idx: %llu key: %.*s(%llu)\n", i, node_data_kv.value.key.len, node_data_kv.value.key.str, node_data_kv.key);
            hmdel(ui_state->node_data, ui_state->node_data[i].key);
        } else {
            ui_state->node_data[i].value.event = (UI_Event){0};
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

    Vector2 mouse_scroll = GetMouseWheelMoveV();
    Vec2 m_scroll = (Vec2){mouse_scroll.x, mouse_scroll.y};
    int scrolled = mouse_scroll.x || mouse_scroll.y;

    int key = 0;

    UI_Mod mod = 0;

    // --- MOD ---

    // TODO: Add alt, ctr, super, (option/cmd)?

    if (IsKeyDown(KEY_LEFT_SHIFT)) mod |= UI_MOD_L_SHIFT;
    if (IsKeyDown(KEY_RIGHT_SHIFT)) mod |= UI_MOD_R_SHIFT;

    // --- MOUSE ---

    if (mouse_moved) arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_MOUSE_MOVE, .pos=m_pos}));

    if (scrolled) arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_SCROLL, .pos=m_pos, .delta=m_scroll}));

    if (IsMouseButtonPressed(0))  arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_MOUSE_LEFT, .pos=m_pos}));
    if (IsMouseButtonPressed(1))  arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_MOUSE_RIGHT, .pos=m_pos}));
    if (IsMouseButtonReleased(0)) arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_RELEASE, .key=UI_MOUSE_LEFT, .pos=m_pos}));
    if (IsMouseButtonReleased(1)) arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_RELEASE, .key=UI_MOUSE_RIGHT, .pos=m_pos}));

    // --- TEXT ---

    if ((key=GetCharPressed()))      arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=key, .mod=mod}));
    if (IsKeyPressed(KEY_BACKSPACE)) arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_BACKSPACE, .mod=mod}));
    if (IsKeyPressed(KEY_DELETE))    arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_DELETE, .mod=mod}));
    if (IsKeyPressed(KEY_TAB))       arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key='\t', .mod=mod}));
    if (IsKeyPressed(KEY_ENTER))     arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key='\n', .mod=mod}));
    if (IsKeyPressed(KEY_LEFT))      arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_LEFT, .mod=mod}));
    if (IsKeyPressed(KEY_RIGHT))     arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_RIGHT, .mod=mod}));
    if (IsKeyPressed(KEY_UP))        arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_UP, .mod=mod}));
    if (IsKeyPressed(KEY_DOWN))      arrpush(ui_state->event_buffer, ((UI_Event){.kind=UI_EVENT_PRESS, .key=UI_DOWN, .mod=mod}));
}

void ui_dispatch_events(void) {
    for (usize i = 0; i < arrlen(ui_state->event_buffer); ++i) {
        UI_Node *current = ui_state->root_node;
        UI_Event ev = ui_state->event_buffer[i];
        UI_Node_Data_KV *data = NULL;

        switch (ev.kind) {
            case UI_EVENT_SCROLL:
                data = hmgetp(ui_state->node_data, ui_state->hovering);
                data->value.event = ev;
                break;
            case UI_EVENT_MOUSE_MOVE:
                do {
                    if (point_in_rect(ev.pos, current->dim)) {
                        if (current->first_child) {
                            current = current->first_child;
                            continue;
                        } else {
                            ui_state->hovering = current->hash;
                            break;
                        }
                    } else {
                        if (!current->next && current->parent)
                            ui_state->hovering = current->parent->hash;
                        current = current->next;
                    }
                } while (current);

                data = hmgetp(ui_state->node_data, (ui_state->mode == UI_MODE_EDIT) ? ui_state->focused : ui_state->hovering);
                data->value.event = ev;
                break;
            case UI_EVENT_PRESS:
                if (ev.key == UI_MOUSE_LEFT || ev.key == UI_MOUSE_RIGHT) {
                    ui_state->focused = ui_state->hovering;
                    ui_state->mode = UI_MODE_NORMAL;
                }
            case UI_EVENT_RELEASE:
                switch (ui_state->mode) {
                    case UI_MODE_NORMAL:
                        data = hmgetp(ui_state->node_data, ui_state->focused);
                        current = data->value.node;
                        if (ev.key == '\t' && !(ev.mod & (UI_MOD_L_SHIFT | UI_MOD_R_SHIFT))) {
                            if (current->first_child) ui_state->focused = current->first_child->hash;
                            else if (current->next) ui_state->focused = current->next->hash;
                            else {
                                do {
                                    if (!current->parent) break;
                                    current = current->parent;
                                } while (!current->next);
                                if (current->next) ui_state->focused = current->next->hash;
                            }
                        } else if (ev.key == '\t' && (ev.mod & (UI_MOD_L_SHIFT | UI_MOD_R_SHIFT))) {
                            if (current->last_child) ui_state->focused = current->last_child->hash;
                            else if (current->prev) ui_state->focused = current->prev->hash;
                            else {
                                do {
                                    if (!current->parent) break;
                                    current = current->parent;
                                } while (!current->prev);
                                if (current->prev) ui_state->focused = current->prev->hash;
                            }

                        } else if (ev.key == '\n') {
                            data = hmgetp(ui_state->node_data, ui_state->focused);
                            data->value.event = ev;
                        } else {
                            data = hmgetp(ui_state->node_data, ui_state->focused);
                            data->value.event = ev;
                        }
                        break;
                    case UI_MODE_EDIT:
                        data = hmgetp(ui_state->node_data, ui_state->focused);
                        data->value.event = ev;
                        break;
                }
                break;
        }
    }

    arrsetlen(ui_state->event_buffer, 0);
}

void ui_push_parent(UI_Node *parent) {
    ui_state->parent = parent;
}

void ui_pop_parent(void) {
    ui_state->parent = ui_state->parent->parent;
}

UI_Node *ui_panel(String id, UI_Flags flags) {
    UI_Node *panel_node = ui_make_node(UI_DRAW_BACKGROUND | UI_LAYOUT_H | flags, id);
    panel_node->size[UI_Axis2_X].kind = UI_Size_Children_Sum;
    panel_node->size[UI_Axis2_Y].kind = UI_Size_Children_Sum;
    return panel_node;
}

UI_Node *ui_label(String label, UI_Flags flags) {
    UI_Node *label_node = ui_make_node(UI_DRAW_TEXT | flags, label);
    label_node->size[UI_Axis2_X].kind = UI_Size_Text_Content;
    label_node->size[UI_Axis2_Y].kind = UI_Size_Text_Content;
    
    return label_node;
}

int ui_button(String label, UI_Flags flags) {
    UI_Node *button_node = ui_make_node(UI_DRAW_TEXT | UI_DRAW_BACKGROUND | UI_DRAW_BORDER | flags, label);
    
    button_node->size[UI_Axis2_X].kind = UI_Size_Text_Content;
    button_node->size[UI_Axis2_Y].kind = UI_Size_Text_Content;
    
    ssize idx = hmgeti(ui_state->node_data, button_node->hash); // idx should never be -1
    assert(idx >= 0);

    UI_Event ev = ui_state->node_data[idx].value.event;
    
    return (ev.kind == UI_EVENT_PRESS && ev.key == UI_MOUSE_LEFT) ||
           (ev.kind == UI_EVENT_PRESS && ev.key == '\n' && ui_state->mode == UI_MODE_NORMAL);
}

char *ui_text_input(String label, UI_Flags flags) {
    UI_Node *text_input = ui_make_node(UI_DRAW_ED_TEXT | UI_DRAW_BACKGROUND | UI_DRAW_BORDER | UI_DRAW_CURSOR | flags, label);

    text_input->size[UI_Axis2_X].kind = UI_Size_Ed_Text_Content;
    text_input->size[UI_Axis2_Y].kind = UI_Size_Ed_Text_Content;

    ssize idx = hmgeti(ui_state->node_data, text_input->hash); // idx should never be -1
    assert(idx >= 0);

    UI_Node_Data_KV *kv = hmgetp(ui_state->node_data, text_input->hash);

    UI_Event ev = kv->value.event;

    // TODO: Move event handling into ui_make_node?? maybe
    if (!(flags & UI_TEXT_NO_ED)) {
        switch (ev.kind) {
            case UI_EVENT_PRESS:
                if (!kv->value.ed_string) arrpush(kv->value.ed_string, 0);
                switch (ev.key) {
                    case UI_BACKSPACE:
                        if ((kv->value.cursor-1) < 0) break;
                        --kv->value.cursor;
                        arrdel(kv->value.ed_string, kv->value.cursor);
                        break;
                    case UI_DELETE:
                        if ((kv->value.cursor+1) > arrlen(kv->value.ed_string)-1) break;
                        arrdel(kv->value.ed_string, kv->value.cursor);
                        break;
                    case UI_MOUSE_LEFT:
                    case UI_MOUSE_RIGHT:
                        ui_state->mode = UI_MODE_EDIT;
                        break;
                    case UI_LEFT:
                        if ((kv->value.cursor-1) < 0) break;
                        --kv->value.cursor;
                        kv->value.mark = kv->value.cursor;
                        break;
                    case UI_RIGHT:
                        if (kv->value.cursor+1 > arrlen(kv->value.ed_string)-1) break;
                        ++kv->value.cursor;
                        kv->value.mark = kv->value.cursor;
                        break;
                    case UI_UP: // TODO: Make these do something
                        break;
                    case UI_DOWN:
                        break;
                    default:
                        arrins(kv->value.ed_string, kv->value.cursor, ev.key);
                        ++kv->value.cursor;
                        break;
                }
                kv->value.mark = kv->value.cursor;
                break;
            default:
                break;
        }
    }
    return kv->value.ed_string;
}

// FIXME: change from iterating over the children to building self with parent as ref, maybe.
void ui_layout(UI_Node *node)
{
    Vector2 text_size;
    UI_Node *child;
    UI_Node_Data_KV *kv;
    if (!node) return;
    
    node->pos_start[UI_Axis2_X] = 0;
    node->pos_start[UI_Axis2_Y] = 0;
    
    UI_Node *parent = node->parent;

    kv = hmgetp(ui_state->node_data, node->hash);
    
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
                f32 xy[UI_Axis2_COUNT] = {text_size.x, text_size.y ? text_size.y : FONT_SIZE};
                node->dim.wh[ax] = xy[ax]+2*node->pad[ax];
                break;
            case UI_Size_Ed_Text_Content: {
                text_size = kv->value.ed_string ? MeasureTextEx(
                                          GetFontDefault(),
                                          (const char*)kv->value.ed_string,
                                          FONT_SIZE,
                                          FONT_SIZE/10
                                          ) : (Vector2){0};
                f32 xy[UI_Axis2_COUNT] = {text_size.x, text_size.y ? text_size.y : FONT_SIZE};
                node->dim.wh[ax] = xy[ax]+2*node->pad[ax];
                break;
            }
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
    int i = 0;

    if (!node) return;
    Rectangle r = {
        .x = node->dim.xy[UI_Axis2_X],
        .y = node->dim.xy[UI_Axis2_Y],
        .width = node->dim.wh[UI_Axis2_X],
        .height = node->dim.wh[UI_Axis2_Y],
    };

    UI_Node_Data_KV *kv = hmgetp(ui_state->node_data, node->hash);

    if (node->hash == ui_state->hovering) ++i;
    if (node->hash == ui_state->focused) ++i;
    
    if (node->flags & UI_DRAW_BACKGROUND)
        DrawRectangleRec(r, ui_state->background_color[i]);
    if (node->flags & UI_DRAW_BORDER)
        DrawRectangleLinesEx(r, 5, ui_state->border_color[i]);
    if (node->flags & UI_DRAW_TEXT)
        DrawText((const char *)node->string.str,
                 node->dim.xy[0]+node->pad[0],
                 node->dim.xy[1]+node->pad[1],
                 FONT_SIZE,
                 ui_state->text_color[i]);
    if (node->flags & UI_DRAW_ED_TEXT && kv->value.ed_string) {
        DrawText((const char *)kv->value.ed_string,
                 node->dim.xy[0]+node->pad[0],
                 node->dim.xy[1]+node->pad[1],
                 FONT_SIZE,
                 ui_state->text_color[i]);
        if (node->flags & UI_DRAW_CURSOR && node->hash == ui_state->focused) {
            char *txt = aprintf(ui_state->temp_arena, "%.*s", kv->value.cursor, kv->value.ed_string);
            int txt_size = MeasureText(txt, FONT_SIZE);
            DrawRectangle(node->dim.xy[0]+node->pad[0]+txt_size,
                          node->dim.xy[1]+node->pad[1],
                          2,
                          FONT_SIZE,
                          ui_state->text_color[i]);
        }
    }

    ui_draw(node->first_child);
    ui_draw(node->next);
}

#endif

#endif
