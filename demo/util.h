#ifndef NEOPAD_DEMO_UTIL_H
#define NEOPAD_DEMO_UTIL_H

#include <GLFW/glfw3.h>

/// Gets the second display (if any), otherwise NULL.
GLFWmonitor *glfwGetStartupMonitor(void);

/// Finds the monitor that the window is mostly on.
/// This is used to target the correct monitor when creating a fullscreen window.
/// @return The monitor that the window is mostly on.
GLFWmonitor *glfwFindWindowMonitor(GLFWwindow* window);

#endif