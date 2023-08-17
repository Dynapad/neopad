// Most of the important includes have configuration, and are included via platform.h.
#include "platform.h"
#include "demo.h"
#include "util.h"

#include <neopad/renderer.h>

/**
 * Demo app state. Everything goes in here.
 */
typedef struct {
    neopad_renderer_t renderer;
    GLFWwindow *window;

    /// Last known cursor position.
    struct {
        vec2 pos;
        bool is_down;
    } cursor;

    /// Last known window size (e.g. to return to after fullscreen)
    struct {

    } windowed;

    struct {
        /// Are we in fullscreen mode?
        bool is_active;

        /// Last known window position and size (e.g. to return to after fullscreen)
        struct {
            ivec2 pos;
            ivec2 size;
        } saved_window;
    } fullscreen;

    struct {
        /// Is the user dragging the mouse?
        bool is_dragging;
        /// Window coordinates of the drag start.
        vec2 from;
        /// Window coordinates of the drag end.
        vec2 to;
        /// Initial camera position to offset from.
        vec2 from_camera;
    } drag;

    struct {
        /// Is the user zooming?
        bool is_zooming;

        /// Current zoom level.
        float level;

        // Maximum and minimum (config).
        float max_level;
        float min_level;

        // Zoom factor (config).
        float scroll_factor;
    } zoom;
} demo_state_t;

#pragma mark - GLFW Callbacks

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
                neopad_renderer_set_camera(state->renderer, (vec2) {0, 0});
                neopad_renderer_zoom(state->renderer, 1);
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
                float zoom = (float) n;
                bool mod_shift = mods & GLFW_MOD_SHIFT;

                if (n == 0) {
                    zoom = mod_shift ? state->zoom.min_level : state->zoom.max_level;
                } else if (n == 1) {
                    zoom = 1;
                } else if (n <= 9) {
                    zoom = mod_shift ? 1.0f / zoom : zoom;
                }

                neopad_renderer_set_camera(state->renderer, (vec2) {0, 0});
                neopad_renderer_zoom(state->renderer, zoom);

                break;
            }
            case GLFW_KEY_F:
                toggle_fullscreen(window);
                break;
            default:
                break;
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        neopad_renderer_arrest_zoom(state->renderer);
        if (action == GLFW_PRESS) {
            state->cursor.is_down = true;

            vec4 viewport;
            get_viewport(window, viewport);
            get_cursor_pos(window, state->drag.from);

            // Convert the cursor position to screen coordinates.
            neopad_renderer_window_to_screen(state->renderer, viewport,
                                             state->drag.from, state->drag.from);

            // Save the starting camera position.
            neopad_renderer_get_camera(state->renderer, state->drag.from_camera);
        } else if (action == GLFW_RELEASE) {
            state->cursor.is_down = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (state->cursor.is_down) {
        get_cursor_pos(window, state->drag.to);

        vec4 viewport;
        get_viewport(window, viewport);
        neopad_renderer_window_to_screen(state->renderer, viewport, state->drag.to, state->drag.from);

        vec2 drag_delta;
        glm_vec2_sub(state->drag.to, state->drag.from, drag_delta);

        vec2 camera;
        glm_vec2_add(state->drag.from_camera, drag_delta, camera);
        neopad_renderer_set_camera(state->renderer, camera);
    }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // Adjust the zoom level logarithmically.
    if (y_offset > 0)
        state->zoom.level *= powf(state->zoom.scroll_factor, (float) y_offset);
    else if (y_offset < 0)
        state->zoom.level /= powf(state->zoom.scroll_factor, (float) -y_offset);

    // Clamp zoom betwixt 0.1 and 10.0.
    state->zoom.level = glm_clamp(state->zoom.level,
                                  state->zoom.min_level,
                                  state->zoom.max_level);
    neopad_renderer_zoom(state->renderer, state->zoom.level);
}

void window_size_callback(GLFWwindow *window, int width, int height) {

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (state->renderer == NULL) return;
    neopad_renderer_resize(state->renderer, width, height);
    draw(window);
}

void window_content_scale_callback(GLFWwindow *window, float xscale, float yscale) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    if (state->renderer == NULL) return;
    printf("Content scale: %f, %f\n", xscale, yscale);
    neopad_renderer_rescale(state->renderer, xscale);
    draw(window);
}

#pragma mark - Setup / Teardown

GLFWwindow *setup_glfw(int width, int height, demo_state_t *state) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        eprintf("Error: unable to initialize GLFW");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    GLFWmonitor *monitor = NULL;
    GLFWwindow *window = glfwCreateWindow(width, height, "Neopad Demo", monitor, NULL);
    if (!window) {
        eprintf("Error: unable to create window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Set to demo state.
    glfwSetWindowUserPointer(window, state);

    // Set the cursor.
    GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, cursor);

    // Set callbacks.
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowContentScaleCallback(window, window_content_scale_callback);

    return window;;
}

void teardown_glfw(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void setup_neopad(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    int width, height;
    float content_scale = 1.0f;

    glfwGetFramebufferSize(window, &width, &height);
    glfwGetWindowContentScale(window, &content_scale, NULL);

    neopad_renderer_init_t init = {
            .width = width,
            .height = height,
            .content_scale = content_scale,
            .debug = true,
            .native_window_handle = demo_get_native_window_handle(window),
            .native_display_type = demo_get_native_display_type(window),
            .background = {
                    .color = 0x222222FF,
                    .grid_enabled = true,
                    .grid_major = 100,
                    .grid_minor = 25,
            }
    };

    state->renderer = neopad_renderer_create();
    neopad_renderer_await_frame(state->renderer, 0); // marks this as the render thread
    neopad_renderer_init(state->renderer, init);  // marks this as the API thread
}

void teardown_neopad(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    neopad_renderer_shutdown(state->renderer);
    neopad_renderer_destroy(state->renderer);
}

void run(GLFWwindow *window) {
    setup_neopad(window);

    while (!glfwWindowShouldClose(window)) {
        draw(window);
        glfwPollEvents();
    }

    teardown_neopad(window);
}

void draw(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    neopad_renderer_begin_frame(state->renderer);
    neopad_renderer_draw_background(state->renderer);
    neopad_renderer_draw_test_rect(state->renderer, -100, 100, 100, -100);
    neopad_renderer_end_frame(state->renderer);
}

void toggle_fullscreen(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // If we're already in fullscreen mode, exit it.
    if (state->fullscreen.is_active) {
        int *pos = &state->fullscreen.saved_window.pos[0];
        int *size = &state->fullscreen.saved_window.size[0];

        neopad_renderer_resize(state->renderer, size[0], size[1]);
        glfwSetWindowMonitor(window, NULL,
                             pos[0], pos[1],
                             size[0], size[1],
                             GLFW_DONT_CARE);
        state->fullscreen.is_active = false;
        return;
    }

    // Otherwise, save the window position and size.
    int *pos = state->fullscreen.saved_window.pos;
    int *size = state->fullscreen.saved_window.size;
    glfwGetWindowPos(window, &pos[0], &pos[1]);
    glfwGetWindowSize(window, &size[0], &size[1]);

    // Get the current monitor and its details.
    GLFWmonitor *monitor = glfwFindWindowMonitor(window); // note: custom function in util.h
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    // Resize the renderer.
    neopad_renderer_resize(state->renderer, mode->width, mode->height);

    // Switch to fullscreen mode.
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    state->fullscreen.is_active = true;
}

int main() {
    demo_state_t state;
    state.zoom.min_level = 0.1f;
    state.zoom.max_level = 10.0f;
    state.zoom.scroll_factor = 1.2f; // 1.0 scroll = 20% zoom

    GLFWwindow *window = setup_glfw(1200, 800, &state);
    run(window);
    teardown_glfw(window);
    return EXIT_SUCCESS;
}