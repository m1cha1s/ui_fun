#ifndef _UTIL_H
#define _UTIL_H

#include <stddef.h>
#include <string.h>

#define memzero(ptr) memset((ptr), 0, sizeof(*(ptr)))

typedef struct Arena_Block Arena_Block;
struct Arena_Block
{
    Arena_Block *next;
    size_t count;    // In words
    size_t capacity; // In words
    size_t memory[];
};

typedef struct 
{
    enum {
       ARENA_STATIC,
       ARENA_GROWING,
    } kind;
    
    size_t default_block_size;
    
    Arena_Block *root, *current;
} Arena;

// This buffer includes both the arena as well as the block header
void static_arena_init(Arean *arena, size_t *buffer, size_t size);
void growing_arena_init(Arean *arena, size_t default_block_size);

void *arena_alloc(Arena *arena, size_t size);
void arena_reset(Arena *arena);
void arena_free(Arena *arena); // This only works on the growing one

#ifdef UTIL_IMPLEMENTATION

void static_arena_init(Arean *arena, size_t *buffer, size_t size)
{
    arena->kind = ARENA_STATIC;
    arena->root = arena->current = (Arena_Block*)buffer;
    arena->root->count = 0;
    arena->root->capacity = size-sizeof(Arena_Block);
}

static void arena_block_alloc(Arena *arena, size_t block_size)
{
    Arena_Block *ab = malloc(block_size+sizeof(Arena_Block));
    memzero(ab);
    ab->capacity = block_size/sizeof(block_size);
    
    if (!arena->root)
    {
        arena->current = arena->root = ab;
    }
    else
    {
        arena->current->next = ab;
        arena->current = ab;
    }
}

void growing_arena_init(Arena *arena)
{
    arena->kind = ARENA_GROWING;
    arena->default_block_size = default_block_size;
    
    arena_block_alloc(arena, arena->default_block_size);
}

void *arena_alloc(Arena *arena, size_t size)
{
    switch (arena->kind)
    {
        case ARENA_STATIC:
        {
        
        } break;
    }
}

void arena_reset(Arena *arena)
{

}

void arena_free(Arena *arena)
{

}

#endif

#endif
