/* date = January 9th 2025 6:51 pm */

#ifndef MS_ARENA_H
#define MS_ARENA_H

#include <stddef.h>

#define b8 char
#define u8 unsigned char

typedef struct Arena_Block {
    struct Arena_Block *next;
    size_t cap;
    size_t end;
    u8 block[];
} Arena_Block;

typedef struct {
    Arena_Block *first;
    Arena_Block *current;
} Arena;

void *ArenaAlloc(Arena *arena, size_t size);
void ArenaReset(Arena *arena);
void ArenaFree(Arena *arena);

#if !defined(ARENA_MALLOC) || !defined(ARENA_FREE)
#include <stdlib.h>
#endif

#ifndef ARENA_MALLOC
#define ARENA_MALLOC(size) malloc((size))
#endif
#ifndef ARENA_FREE
#define ARENA_FREE(ptr) free((ptr))
#endif

#ifndef MS_ARENA_MIN_CAP
#define MS_ARENA_MIN_CAP 4096
#endif

#ifdef MS_ARENA_IMPLEMENTATION
#undef MS_ARENA_IMPLEMENTATION

static Arena_Block *NewArenaBlock(size_t cap)
{
    size_t s = sizeof(Arena_Block) + cap;
    Arena_Block *block = (Arena_Block*)ARENA_MALLOC(s);
    block->next = NULL;
    block->cap = cap;
    block->end = 0;
    return block;
}

static void FreeArenaBlock(Arena_Block *block)
{
    if (block->next) FreeArenaBlock(block);
    ARENA_FREE(block);
}

void *ArenaAlloc(Arena *arena, size_t size)
{
    if (!arena->first || !arena->current)
    {
        arena->first = arena->current = NewArenaBlock(MS_ARENA_MIN_CAP);
    }
    
    if (arena->current->end + size > arena->current->cap)
    {
        arena->current->next = NewArenaBlock(size > MS_ARENA_MIN_CAP ? size : MS_ARENA_MIN_CAP);
        arena->current = arena->current->next;
    }
    
    void *reg = &arena->current->block[arena->current->end];
    arena->current->end += size;
    
    return reg;
}

void ArenaReset(Arena *arena)
{
	if (!arena->first) return;
	arena->current = arena->first;
	for (;arena->current->next;)
	{
		arena->current->end = 0;
		arena->current = arena->current->next;
	}
}

void ArenaFree(Arena *arena)
{
	if (!arena->first) return;
    FreeArenaBlock(arena->first);
}

#endif

#undef b8 
#undef u8

#endif //ARENA_H
