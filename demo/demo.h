//
// Created by Dylan Lukes on 8/16/23.
//

#ifndef NEOPAD_DEMO_H
#define NEOPAD_DEMO_H

#include "platform.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
void draw(GLFWwindow *window);
void toggle_fullscreen(GLFWwindow *window);

#endif //NEOPAD_DEMO_H
