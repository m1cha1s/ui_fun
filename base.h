#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <stddef.h>

// ==== TYPES ==== 

typedef int8_t   s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;
typedef ptrdiff_t ssize;

typedef float f32;
typedef double f64;

typedef u8   b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

typedef struct Vec2
{
    f32 x, y;
} Vec2;

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct String {
    u8 *str;
    usize len;
} String;
#define S(str) ((String){(u8*)str, sizeof(str)-1})

#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define ArrayLen(arr) (sizeof((arr))/sizeof(*(arr)))

void memory_set(void *ptr, u8 val, usize size);

typedef struct Arena_Block {
    struct Arena_Block *next;
    usize cap;
    usize end;
    u8 block[];
} Arena_Block;

typedef struct {
    Arena_Block *first;
    Arena_Block *current;
} Arena;

Arena *arena_new(void);
void *arena_alloc(Arena *arena, size_t size);
void arena_reset(Arena *arena);
void arena_free(Arena *arena);

#include <stdarg.h>
#include <string.h>

char *aprintf(Arena *a, char *fmt, ...);

#if !defined(ARENA_MALLOC) || !defined(ARENA_FREE)
#include <stdlib.h>
#endif

#ifndef ARENA_MALLOC
#define ARENA_MALLOC(size) malloc((size))
#endif
#ifndef ARENA_FREE
#define ARENA_FREE(ptr) free((ptr))
#endif

#ifndef BASE_ARENA_MIN_CAP
#define BASE_ARENA_MIN_CAP 4096
#endif

#endif // _TYPES_H

#ifdef BASE_IMPLEMENTATION
#undef BASE_IMPLEMENTATION

void memory_set(void *ptr, u8 val, usize size) {
    u8 *p = (u8*)ptr;
    for (usize s = 0; s < size; ++s) p[s] = val;
}

Arena *arena_new(void) {
    Arena *arena = ARENA_MALLOC(sizeof(Arena));
    memory_set(arena, 0, sizeof(Arena));
    return arena;
}

static Arena_Block *new_arena_block(size_t cap)
{
    printf("new_arena_block\n");
    size_t s = sizeof(Arena_Block) + cap;
    Arena_Block *block = (Arena_Block*)ARENA_MALLOC(s);
    block->next = NULL;
    block->cap = cap;
    block->end = 0;
    return block;
}

static void free_arena_block(Arena_Block *block)
{
    printf("free_arena_block\n");
    if (block->next) free_arena_block(block->next);
    ARENA_FREE(block);
}

void *arena_alloc(Arena *arena, size_t size)
{
    if (!arena->first || !arena->current)
    {
        arena->first = arena->current = new_arena_block(BASE_ARENA_MIN_CAP);
    }
    
    do {
        if (!arena->current->next) {
            printf("arena_alloc: no next\n");
            arena->current->next = new_arena_block(size > BASE_ARENA_MIN_CAP ? size : BASE_ARENA_MIN_CAP);
        }
        arena->current = arena->current->next;
    } while ((arena->current->end + size) > arena->current->cap);
    
    void *reg = &arena->current->block[arena->current->end];
    arena->current->end += size;
    
    return reg;
}

void arena_reset(Arena *arena)
{
    // printf("arena_reset\n");
	if (!arena->first) return;
	arena->current = arena->first;
    while (arena->current->next)
    {
        arena->current->end = 0;
        arena->current = arena->current->next;
    }
    arena->current = arena->first;
}

void arena_free(Arena *arena)
{
	if (!arena->first) return;
    free_arena_block(arena->first);
    ARENA_FREE(arena);
}

char *aprintf(Arena *a, char *fmt, ...) {
    va_list args;
    ssize buf_size;
    char *buf;
    
    va_start(args, fmt);
    buf_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    buf = arena_alloc(a, buf_size+1);
    
    va_start(args, fmt);
    vsnprintf(buf, buf_size+1, fmt, args);
    va_end(args);
    
    buf[buf_size] = 0;
    
    return buf;
}

#endif // BASE_IMPLEMENTATION
