#ifndef _UI_H
#define _UI_H

#include <stdarg.h>

#include "base.h"

typedef struct UI_Key {
    String8 literal;
    u32 hash;
} UI_Key;

typedef struct Rng2f32 {
    f32 x1, y1, x2, y2;
} Rng2f32;

typedef enum UI_SizeKind {
    UI_SizeKind_NULL,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
} UI_SizeKind;

typedef struct UI_Size {
    UI_SizeKind kind;
    f32 value;
    f32 strictness;
} UI_Size;

typedef enum Axis2 {
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT,
} Axis2;

typedef u32 UI_NodeFlags;
enum {
    UI_NodeFlag_Clickable = (1<<0),
    UI_NodeFlag_ViewScroll = (1<<1),
    UI_NodeFlag_DrawText = (1<<2),
    UI_NodeFlag_DrawBorder = (1<<3),
    UI_NodeFlag_DrawBackground = (1<<4),
    UI_NodeFlag_DrawDropShadow = (1<<5),
    UI_NodeFlag_Clip = (1<<6),
    UI_NodeFlag_HotAnimation = (1<<7),
    UI_NodeFlag_ActiveAnimation = (1<<8),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    UI_Node *first;
    UI_Node *last;
    UI_Node *next;
    UI_Node *prev;
    UI_Node *parent;

    // Per-frame info provided by builders
    UI_NodeFlags flags;
    String8 string;
    UI_Size semantic_size[Axis2_COUNT];

    // Computed every frame
    f32 computed_rel_position[Axis2_COUNT];
    f32 computed_size[Axis2_COUNT];
    Rng2f32 rect;

    UI_Node *hash_next;
    UI_Node *hash_prev;

    UI_Key key;
    u64 last_frame_touched_index;

    // persistant
    f32 hot_t;
    f32 active_t;
};

typedef struct UI_Comm {
    UI_Node *node;
    Vec2f32 mouse;
    Vec2f32 drag_delta;
    b8 clicked;
    b8 double_clicked;
    b8 right_clicked;
    b8 pressed;
    b8 released;
    b8 dragging;
    b8 hovering;
} UI_Comm;

typedef struct UI_State {
    Arena *arena;

    UI_Node *hash_first, *hash_last;
} UI_State;
extern UI_State *ui_state;

void UI_InitState(UI_State *state);
void UI_SetState(UI_State *state);

void UI_BeginFrame(void);

// Helpers
UI_Key UI_KeyNull(void);
UI_Key UI_KeyFromString(String8 string);
b32 UI_KeyMatch(UI_Key a, UI_Key b);

UI_Node *UI_NodeMake(UI_NodeFlags flags, String8 string);
UI_Node *UI_NodeMakeF(UI_NodeFlags flags, char *fmt, ...);

void UI_NodeEquipDisplayString(UI_Node *node, String8 string);
// void UI_NodeEquipChildLayoutAxis(UI_Node *node, Axis2 axis);

UI_Node *UI_PushParent(UI_Node *node);
UI_Node *UI_PopParent(void);

UI_Comm UI_CommFromNode(UI_Node *node);

#ifdef IMPL

UI_State *ui_state;
u64 ui_frame_index;

void UI_InitState(UI_State *state) {
    MemZeroPtr(state);
}

void UI_SetState(UI_State *state) {
    ui_state = state;
}

void UI_BeginFrame(void) {
    ui_frame_index += 1;
}

// Helpers
UI_Key UI_KeyNull(void) {
    return (UI_Key){0};
}

UI_Key UI_KeyFromString(String8 string) {
    return (UI_Key){
        .literal = string,
        .hash = Hash_String8(string),
    };
}

b32 UI_KeyMatch(UI_Key a, UI_Key b) {
    return a.hash == b.hash; // This may not be correct...
}




UI_Node *UI_NodeMake(UI_NodeFlags flags, String8 string) {
    UI_Node *node = ArenaAlloc(ui_state->arena, sizeof(UI_Node));

    MemZeroPtr(node);

    node->flags = flags;
    node->key = UI_KeyFromString(string);

    if (!ui_state->hash_first || !ui_state->hash_last) {
        ui_state->hash_first = ui_state->hash_last = node;
    } else {
        ui_state->hash_last->hash_next = node;
        node->prev = ui_state->hash_last;
        ui_state->hash_last = node;
    }

    return node;
}

// TODO: Implement the formatting
UI_Node *UI_NodeMakeF(UI_NodeFlags flags, char *fmt, ...) {
    String8 str = String8_Slice(fmt, CStrLen(fmt));
    return UI_NodeMake(flags, str);
}


void UI_NodeEquipDisplayString(UI_Node *node, String8 string) {
    node->string = string;
}

// void UI_NodeEquipChildLayoutAxis(UI_Node *node, Axis2 axis) {
//     node->
// }


UI_Node *UI_PushParent(UI_Node *node) {

}

UI_Node *UI_PopParent(void) {

}


UI_Comm UI_CommFromNode(UI_Node *node) {

}

#endif

#endif