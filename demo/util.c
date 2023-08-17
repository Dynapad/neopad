#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "cglm/cglm.h"
#include "neopad/types.h"

GLFWmonitor *glfwGetStartupMonitor(void) {
    // Get the list of monitors.
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    // Return the second monitor if it exists, otherwise NULL (start windowed).
    return monitor_count > 1 ? monitors[1] : NULL;
}

void get_cursor_pos(GLFWwindow *window, neopad_vec2_t *dest) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    dest->x = (float) x;
    dest->y = (float) y;
}

void get_viewport(GLFWwindow *window, neopad_vec4_t *dest) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    dest->left = 0;
    dest->top = 0;
    dest->right = (float) w;
    dest->bottom = (float) h;
}


