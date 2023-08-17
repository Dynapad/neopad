//
// Created by Dylan Lukes on 8/16/23.
//

#include "demo.h"
#include "util.h"

void error_callback(int error, const char *description) {
    eprintf("GLFW Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_SPACE:
                state->camera.x = 0;
                state->camera.y = 0;
                state->is_dirty.camera = true;

                state->zoom.level = 1;
                state->is_dirty.zoom = true;
                break;
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_5:
            case GLFW_KEY_6:
            case GLFW_KEY_7:
            case GLFW_KEY_8:
            case GLFW_KEY_9:
            case GLFW_KEY_0: {
                // The numeric keys correspond to zoom level "stops" above and below 1:1.
                //
                // There are 17 stops, arranged like:
                // 0, 9, 8, ... , 3, 2, 1/Shift-1, Shift-2, Shift-3, ..., Shift-8, Shift-9, Shift-0
                //
                // To zoom in, use the numbers 2-9, with 0 zooming all the way in.
                // To zoom out, use the numbers 2-9, with 0 zooming all the way out.
                // Both 1 and Shift-1 behave identically.

                int n = key - GLFW_KEY_0;
                bool mod_shift = mods & GLFW_MOD_SHIFT;

                float zoom;
                if (n == 0) {
                    zoom = mod_shift ? state->zoom.min_level : state->zoom.max_level;
                } else if (n == 1) {
                    zoom = 1;
                } else if (n <= 9) {
                    zoom = mod_shift ? 1.0f / (float) n : (float) n;
                } else {
                    // unreachable, but handle gracefully
                    zoom = state->zoom.level;
                }

                state->camera.x = state->camera.y = 0;
                state->is_dirty.camera = true;

                state->zoom.level = zoom;
                state->is_dirty.zoom = true;

                break;
            }
            case GLFW_KEY_F:
                // Toggle fullscreen.
                state->fullscreen.is_active = !state->fullscreen.is_active;
                state->is_dirty.fullscreen = true;
                break;
            default:
                break;
        }
    }
}


void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            state->cursor.is_down = true;

            neopad_vec4_t viewport;
            get_viewport(window, &viewport);

            neopad_vec2_t cursor_pos;
            get_cursor_pos(window, &cursor_pos);

            // todo: is viewport.vec[0] going from 0 to 4.29259...?
            // Save the cursor position converted to screen coords.
            neopad_renderer_window_to_screen(state->renderer, viewport, cursor_pos, &state->drag.from);

            // Save the starting camera position.
            neopad_renderer_get_camera(state->renderer, &state->drag.from_camera);
        } else if (action == GLFW_RELEASE) {
            state->cursor.is_down = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (state->cursor.is_down) {
        get_cursor_pos(window, &state->drag.to);

        neopad_vec4_t viewport;
        get_viewport(window, &viewport);
        neopad_renderer_window_to_screen(
                state->renderer, viewport,
                state->drag.to, &state->drag.to
        );

        vec2 drag_delta; // in screen coords
        glm_vec2_sub(state->drag.to.vec, state->drag.from.vec, drag_delta);
        glm_vec2_add(state->drag.from_camera.vec, drag_delta, state->camera.vec);
        state->is_dirty.camera = true;
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    float zoom = state->zoom.level;

    // Adjust the zoom level logarithmically.
    if (y_offset > 0)
        zoom *= powf(state->zoom.scroll_factor, (float) y_offset);
    else if (y_offset < 0)
        zoom /= powf(state->zoom.scroll_factor, (float) -y_offset);

    // Clamp the zoom level within the min/max.
    zoom = glm_clamp(zoom, state->zoom.min_level, state->zoom.max_level);

    state->zoom.level = zoom;
    state->is_dirty.zoom = true;
}

void window_size_callback(GLFWwindow *window, int width, int height) {

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    glm_ivec2_copy((ivec2) {width, height}, state->size.vec);
    state->is_dirty.size = true;
    draw(window); // force draw now for a smooth resize (no jump on release)
}

void content_scale_callback(GLFWwindow *window, float xscale, float yscale) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    state->content_scale = xscale;
    state->is_dirty.content_scale = true;
    draw(window); // force draw now
}