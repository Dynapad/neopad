#ifndef NEOPAD_DEMO_UTIL_H
#define NEOPAD_DEMO_UTIL_H

#include "GLFW/glfw3.h"
#include "cglm/vec2.h"
#include "cglm/vec4.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

/// Gets the second display (if any), otherwise NULL.
GLFWmonitor *glfwGetStartupMonitor(void);

/// Finds the monitor that the window is mostly on.
/// This is used to target the correct monitor when creating a fullscreen window.
/// @return The monitor that the window is mostly on.
GLFWmonitor *glfwFindWindowMonitor(GLFWwindow* window);

/// @return The current cursor position in screen coordinates as a vec2.
void get_cursor_pos(GLFWwindow *window, vec2 dest);

/// @return The current viewport in screen coordinates as a vec4.
void get_viewport(GLFWwindow *window, vec4 dest);

#endif