/// General purpose type definitions used everywhere.

#ifndef NEOPAD_TYPES_H
#define NEOPAD_TYPES_H

#include "cglm/vec4.h"

/// @brief A 4D vector, used for colors, positions, etc.
/// Provides several different ways to access the same data,
/// depending on what makes the most sense for the context.
///
/// @note the `w` component is aliased to `zoom` for convenience.
typedef union {
    vec4 vec;
    struct {
        float x;
        float y;
        float z;
        union {
            float w;
            float zoom;
        };
    };
    struct {
        struct {
            float x;
            float y;
        } pos;
        struct {
            float w;
            float h;
        } size;
    };
    struct {
        vec2 vpos;
        vec2 vsize;
    };
} neopad_vec_t;

/// @brief Like neopad_vec_t, but integers.
typedef union {
    ivec4 vec;
    struct {
        int32_t x;
        int32_t y;
        int32_t z;
        int32_t w;
    };
    struct {
        struct {
            int32_t x;
            int32_t y;
        } pos;
        struct {
            int32_t w;
            int32_t h;
        } size;
    };
    struct {
        ivec2 vpos;
        ivec2 vsize;
    };
} neopad_ivec_t;

typedef struct {
    neopad_vec_t position;
    uint32_t color;
} neopad_vertex_t;

#endif