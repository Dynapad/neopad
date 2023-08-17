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

GLFWmonitor *glfwFindWindowMonitor(GLFWwindow* window) {
    // Get the window's position and size.
    neopad_ivec_t window_rect;
    glfwGetWindowPos(window, &window_rect.pos.x, &window_rect.pos.y);
    glfwGetWindowSize(window, &window_rect.size.w, &window_rect.size.h);

    // Get the list of monitors.
    int monitor_count;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

    // Get the monitor rects.
    neopad_ivec_t monitor_rects[monitor_count];
    for (int i = 0; i < monitor_count; ++i) {
        int x, y;
        glfwGetMonitorPos(monitors[i], &x, &y);
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        monitor_rects[i].pos.x = x;
        monitor_rects[i].pos.y = y;
        monitor_rects[i].size.w = mode->width;
        monitor_rects[i].size.h = mode->height;
    }

    // Calculate the overlaps for all monitors.
    int overlaps[monitor_count];

    for (int i = 0; i < monitor_count; ++i) {
        neopad_ivec_t monitor_rect = monitor_rects[i];

        ivec2 delta_pos, size_sum, sum_minus_delta;
        glm_ivec2_sub(window_rect.vpos, monitor_rect.vpos, delta_pos);
        glm_ivec2_abs(delta_pos, delta_pos);
        glm_ivec2_add(window_rect.vsize, monitor_rect.vsize, size_sum);
        glm_ivec2_sub(size_sum, delta_pos, sum_minus_delta);

        // Calculate the overlap with clamping.
        ivec2 overlap, zero = {0, 0};
        glm_ivec2_sub(size_sum, delta_pos, overlap);
        glm_ivec2_maxv(overlap, zero, overlap);
        glm_ivec2_minv(overlap, sum_minus_delta, overlap);

        // Calculate the total overlap.
        overlaps[i] = overlap[0] * overlap[1];
    }

    // Find the maximum overlap.
    int best_overlap = 0;
    GLFWmonitor *best_monitor = glfwGetPrimaryMonitor();

    for (int i = 0; i < monitor_count; ++i) {
        if (overlaps[i] > best_overlap) {
            best_overlap = overlaps[i];
            best_monitor = monitors[i];
        }
    }

    return best_monitor;
}

void get_cursor_pos(GLFWwindow *window, float *dest) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    dest[0] = (float) x, dest[1] = (float) y;
}

void get_viewport(GLFWwindow *window, float *dest) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    dest[0] = 0, dest[1] = 0, dest[2] = (float) w, dest[3] = (float) h;
}


