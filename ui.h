#ifndef _UI_H
#define _UI_H

#include <stddef.h>
//#include <stdint.h>
#include <string.h>

typedef struct String {
    char *str;
    size_t len;
} String;

typedef enum UI_Axis2 {
    UI_Axis2_x,
    UI_Axis2_y,
    UI_Axis2_COUNT,
} UI_Axis2;

typedef struct Rect {
    float xy[2], wh[2];
} Rect;

typedef enum UI_Sem_Size {
    UI_Sem_Size_Fill,
    UI_Sem_Size_Parent_Percent,
    UI_Sem_Size_Text_Fit;
} UI_Sem_Size;

typedef int UI_Flags;
enum {
    UI_CENTER_X     = (1<<0),
    UI_CENTER_Y     = (1<<1),
    UI_ALIGN_LEFT   = (1<<2),
    UI_ALIGN_RIGHT  = (1<<3),
    UI_ALIGN_TOP    = (1<<4),
    UI_ALIGN_BOTTOM = (1<<5),

    UI_DRAW_TEXT       = (1<<6),
    UI_DRAW_BACKGROUND = (1<<7),
    UI_DRAW_FRAME      = (1<<8),
};

typedef struct UI_Node UI_Node;
struct UI_Node {
    UI_Node *first_child;
    UI_Node *next;

    size_t child_count;

    UI_Sem_Size sem_size[UI_Axis2_COUNT];
    // float desired_size[UI_Axis2_COUNT];

    UI_Flags flags;

    char *string;

    Rect dim;
};

size_t hash_string(char *str, size_t len);
#define hash_String(str) hash_string((str).str, (str).len)

void ui_layout(UI_Node *node);

#ifdef IMPL

size_t hash_string(char *str, size_t len) {
    size_t hash = 2166136261u;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (unsigned char)str[i];
        hash *= 16777619;
    }
    return hash;
}

void ui_layout(UI_Node *root)
{
    UI_Node *child = root->first_child;
    for (size_t i = 0; i < root->child_count; ++i) {
        for (int ax = UI_Axis2_x; ax < UI_Axis2_COUNT; ++ax)
        {
            // 1st: Sizing
            switch (child->sem_size[ax])
            {
            case UI_Sem_Size_Fill:
                child->dim.wh[ax] = root->dim.wh[ax]/root->child_count;
                break;
            // TODO: Add more...
            default:
                break;
            }
            // 2nd: Positioning

        }
        child = child->next;
    }
}

#endif

#endif
