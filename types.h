#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>
#include <stddef.h>

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

typedef struct String {
    u8 *str;
    usize len;
} String;
#define S(str) ((String){(u8*)str, sizeof(str)-1})

#endif