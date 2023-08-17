#ifndef NEOPAD_DEMO_UTIL_H
#define NEOPAD_DEMO_UTIL_H

#include "GLFW/glfw3.h"
#include "cglm/vec2.h"
#include "cglm/vec4.h"
#include "neopad/types.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

/// Gets the second display (if any), otherwise NULL.
GLFWmonitor *glfwGetStartupMonitor(void);

/// @return The current cursor position in screen coordinates as a vec2.
void get_cursor_pos(GLFWwindow *window, neopad_vec2_t *dest);

/// @return The current viewport in screen coordinates as a vec4.
void get_viewport(GLFWwindow *window, neopad_vec4_t *dest);

#endif