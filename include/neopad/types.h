/// General purpose type definitions used everywhere.

#ifndef NEOPAD_TYPES_H
#define NEOPAD_TYPES_H

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/vec4.h>
#include <cglm/ivec2.h>
#include <cglm/ivec3.h>
#include <cglm/ivec4.h>

#define IF_2_Z(...)
#define IF_3_Z(...) __VA_ARGS__
#define IF_4_Z(...) __VA_ARGS__

#define IF_2_W(...)
#define IF_3_W(...)
#define IF_4_W(...) __VA_ARGS__

#define IF_2_POS(...) __VA_ARGS__
#define IF_3_POS(...) __VA_ARGS__
#define IF_4_POS(...) __VA_ARGS__

#define IF_2_SIZE(...) __VA_ARGS__
#define IF_3_SIZE(...)
#define IF_4_SIZE(...)

#define IF_2_DEPTH(...)
#define IF_3_DEPTH(...) __VA_ARGS__
#define IF_4_DEPTH(...) __VA_ARGS__

#define IF_2_VEC_PAIR(...)
#define IF_3_VEC_PAIR(...)
#define IF_4_VEC_PAIR(...) __VA_ARGS__

#define IF_2_RECT(...)
#define IF_3_RECT(...)
#define IF_4_RECT(...) __VA_ARGS__

#define NEOPAD_VEC_TYPE(__glm, __n) neopad_##__glm##__n##_t

#define NEOPAD_VEC_TYPE_DECL(__scalar, __glm, __n)         \
    typedef union {                                        \
        /* As a CGLM vector. */                            \
        __glm##__n vec;                                    \
        /* As a pair of CGLM vectors. */                   \
        IF_##__n##_VEC_PAIR(struct {                       \
            __glm##2 pos_vec2;                             \
            __glm##2 size_vec2;                            \
        };)                                                \
        /* vec2/3/4: (x, y, [z, [w]]) */                   \
        struct {                                           \
            __scalar x;                                    \
            __scalar y;                                    \
            IF_##__n##_Z(union {                           \
                __scalar z;                                \
                __scalar width;                            \
            });                                            \
            IF_##__n##_W(union {                           \
                __scalar w;                                \
                __scalar height;                           \
                __scalar zoom;                             \
            });                                            \
        };                                                 \
        /* vec2/3: (width, height, [depth]) */             \
        IF_##__n##_SIZE(struct {                           \
            __scalar width;                                \
            __scalar height;                               \
            IF_##__n##_DEPTH(__scalar depth;)              \
        };)                                                \
        /* vec4: (left, top, right, bottom) */             \
        IF_##__n##_RECT(struct {                           \
            __scalar left;                                 \
            __scalar top;                                  \
            __scalar right;                                \
            __scalar bottom;                               \
        };)                                                \
    } NEOPAD_VEC_TYPE(__glm, __n);                         \
    /* Avoid any gotchas with alignment or padding. */     \
    _Static_assert(                                        \
        sizeof(neopad_##__glm##__n##_t) == sizeof(__glm##__n), \
        "neopad_" #__glm #__n "_t is not the same size as " #__glm #__n \
    )

NEOPAD_VEC_TYPE_DECL(int, ivec, 2);
NEOPAD_VEC_TYPE_DECL(int, ivec, 3);
NEOPAD_VEC_TYPE_DECL(int, ivec, 4);
NEOPAD_VEC_TYPE_DECL(float, vec, 2);
NEOPAD_VEC_TYPE_DECL(float, vec, 3);
NEOPAD_VEC_TYPE_DECL(float, vec, 4);

///* vec4 (x, y, width, height) */                   \
//IF_##_width##_POS_SIZE(struct {                    \
//    __scalar x;                                    \
//    __scalar y;                                    \
//    __scalar width;                                \
//    __scalar height;                               \
//};)                                                \

typedef union {
    uint32_t abgr;
    struct {
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
} neopad_color_t;

typedef struct {
    neopad_vec4_t position;
    neopad_color_t color;
} neopad_vertex_t;

#endif