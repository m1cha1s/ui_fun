#ifndef _BASE_H
#define _BASE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef u8 b8;
typedef u32 b32;

typedef void(VoidFunc)(void);

#ifndef NULL
#define NULL ((void*)0)
#endif

#define _STR(s) #s
#define STR(s) _STR(s)

#define Stmt(e) do { e } while(0)
#define Crash() ((*(volatile int*)0) = 0)
#define Assert(exp) Stmt( if (!(exp)) { printf("Assertion Failed: %s : %s:%d\n", STR(exp), __FILE__, __LINE__); Crash(); } )
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

void MemZero(void *mem, u64 size);
#define MemZeroPtr(ptr) MemZero((ptr), sizeof(*ptr))

u64 CStrLen(char *cstr);

typedef struct Vec2f32 {
    f32 x, y;
} Vec2f32;

typedef struct String8 {
    u8 *str;
    u64 size;
} String8;

#define String8_Lit(str) ((String8){.str=str, .size=(sizeof(str)-1)})
#define String8_Slice(cstr, len) ((String8){.str=cstr, .size=len})

u32 Hash_String8(String8 str);

typedef struct ArenaBlock ArenaBlock;
struct ArenaBlock {
    u8 *block;
    u64 size;
    u64 end;

    ArenaBlock *next;
};

typedef struct Arena {
    ArenaBlock *first;
    ArenaBlock *current;
} Arena;

#ifndef ARENA_MIN_SIZE
#define ARENA_MIN_SIZE 4096
#endif

void *ArenaAlloc(Arena *arena, u64 size);
void ArenaReset(Arena *arena);
void ArenaFree(Arena *arena);

#define ArenaScope(arena_ptr) DeferLoop(0, ArenaReset((arena_ptr)))

extern Arena *temp_arena;

/* TODO Implement the stb stuff */

// typedef struct ArrayHeader {
//     u64 capacity;
//     u64 length;
// } ArrayHeader;

// #define ArrayHdr(a) ((a) ? ((ArrayHeader*)a)-1 : 0)
// #define ArrayLen(a) ((a) ? ArrayHdr(a)->length : 0)
// #define ArrayCap(a) ((a) ? ArrayHdr(a)->capacity : 0)
// #define ArraySetLen(a,n) ((a) ? ArrayHdr(a)->length=(n) : 0)
// #define ArraySetCap(a,n) ((a) ? ArrayHdr(a)->capacity=(n) : 0)



// #define ArrayMaybeGrow(a,n) ((!(a) || ArrayLen(a) + (n) > ArrayCap(a)) ? (ArrayGrow(a,n,0),0) : 0)

// #define ArrayGrow(a,b,c) ((a) = ArrayGrowWrapper((a), sizeof(*(a) , (b), (c)))

// void ArrayGrowWrapper(void *a, );

#ifdef IMPL

Arena *temp_arena = NULL;

void MemZero(void *mem, u64 size) {
    u8 *m = (u8*)mem;

    for (u64 i = 0; i < size; ++i) m[i] = 0;
}

u64 CStrLen(char *cstr) {
    u64 size = 0;
    while (*(cstr++));
    return size;
}

u32 Hash_String8(String8 str) {
    u32 hash = 2166136261u;
    for (u64 i = 0; i < str.size; ++i) {
        hash ^= (u8)str.str[i];
        hash *= 16777619;
    }
    return hash;
}

void *ArenaAlloc(Arena *arena, u64 size) {
    if (!arena) {
        arena = malloc(sizeof(Arena));
        Assert(arena);
    }
    if (arena->current == 0 || arena->first == 0) {
        arena->current = arena->first = malloc(sizeof(ArenaBlock));
        MemZero(arena->first, sizeof(ArenaBlock));
        Assert(arena->first);
        arena->first->block = malloc(ARENA_MIN_SIZE);
        Assert(arena->first->block);
    }

    while (arena->current->end + size >= arena->current->size) {
        if (!arena->current->next) {
            u64 block_size = sizeof(ArenaBlock)+(size > ARENA_MIN_SIZE ? size : ARENA_MIN_SIZE);
            arena->current->next = malloc(block_size);
            Assert(arena->current->next);
            MemZero(arena->current->next, block_size);
        }
        arena->current = arena->current->next;
    }

    void *ret_ptr = arena->current->block+arena->current->end;
    arena->current->end += size;
    return ret_ptr;
}

static void ArenaBlockReset(ArenaBlock *ab) {
    if (!ab) return;
    ab->end = 0;
    ArenaBlockReset(ab->next);
}

void ArenaReset(Arena *arena) {
    ArenaBlockReset(arena->first);
}

static void ArenaBlockFree(ArenaBlock *ab) {
    if (!ab) return;

    ArenaBlockFree(ab->next);

    free(ab->next);
}

void ArenaFree(Arena *arena) {
    ArenaBlockFree(arena->first);
    free(arena->first);
}

#endif

#endif