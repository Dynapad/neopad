//
// Created by Dylan Lukes on 8/13/23.
//

#ifndef NEOPAD_OBJECT_H
#define NEOPAD_OBJECT_H

#include <cglm/vec2.h>

typedef struct line_s {
    vec2 start;
    vec2 end;
} line_t;

typedef struct rect_s {
    vec2 min;
    vec2 max;
} rect_t;

typedef struct ellipse_s {
    vec2 center;
    vec2 radii;
} ellipse_t;

#endif //NEOPAD_OBJECT_H
