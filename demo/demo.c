// Most of the important includes have configuration, and are included via platform.h.
#include "platform.h"
#include "demo.h"
#include "util.h"

#include <neopad/renderer.h>

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

    // Get the window size and content scale.
    glfwGetFramebufferSize(window, &state->size.width, &state->size.height);
    glfwGetWindowContentScale(window, &state->content_scale, NULL);

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
    glfwSetWindowContentScaleCallback(window, content_scale_callback);

    return window;;
}

void teardown_glfw(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void setup_neopad(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    neopad_renderer_init_t init = {
            .width = state->size.width,
            .height = state->size.height,
            .content_scale = state->content_scale,
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

void enter_fullscreen(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // Save the window position and size.
    neopad_ivec4_t *saved_window = &state->fullscreen.saved_window;
    glfwGetWindowPos(window, &saved_window->x, &saved_window->y);
    glfwGetWindowSize(window, &saved_window->width, &saved_window->height);

    // Get the current monitor and its details.
    GLFWmonitor *monitor = glfwGetStartupMonitor(); // note: custom function in util.h
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    // Resize the renderer.
    state->size.width = mode->width;
    state->size.height = mode->height;
    state->is_dirty.size = true;
    draw(window); // force draw now for nicer transition

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    state->is_dirty.fullscreen = false;
}

void exit_fullscreen(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    neopad_ivec4_t *saved_window = &state->fullscreen.saved_window;

    neopad_renderer_resize(state->renderer, saved_window->width, saved_window->height);
    glfwSetWindowMonitor(window, NULL,
                         saved_window->x, saved_window->y,
                         saved_window->width, saved_window->height,
                         GLFW_DONT_CARE);
    state->is_dirty.fullscreen = false;
}

void run(GLFWwindow *window) {
    setup_neopad(window);

    while (!glfwWindowShouldClose(window)) {
        update(window);
        draw(window);
        glfwPollEvents();
    }

    teardown_neopad(window);
}

void update(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // Early exit if nothing is dirty.
    if (state->is_dirty.any == 0) return;

    // Check fullscreen first, as it may affect other flags.
    if (state->is_dirty.fullscreen) {
        if (state->fullscreen.is_active) {
            enter_fullscreen(window);
        } else {
            exit_fullscreen(window);
        }
    }

    // Check each dirty flag and update the renderer correspondingly.
    if (state->is_dirty.size) {
        neopad_renderer_resize(state->renderer, state->size.width, state->size.height);
        state->is_dirty.size = false;
    }

    if (state->is_dirty.content_scale) {
        neopad_renderer_rescale(state->renderer, state->content_scale);
        state->is_dirty.content_scale = false;
    }

    if (state->is_dirty.camera) {
        neopad_renderer_set_camera(state->renderer, state->camera);
        state->is_dirty.camera = false;
    }

    if (state->is_dirty.zoom) {
        neopad_renderer_zoom(state->renderer, state->zoom.level);
        state->is_dirty.zoom = false;
    }
}

void draw(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);
    neopad_renderer_t renderer = state->renderer;

    // Draw a frame.
    neopad_renderer_begin_frame(renderer);
    neopad_renderer_draw_background(renderer);
    neopad_renderer_draw_test_rect(renderer, -100, 100, 100, -100);
    neopad_renderer_end_frame(renderer);

    // Save the resulting camera position.
    neopad_renderer_get_camera(renderer, &state->camera);
}

const int W = 1200;
const int H = 800;

int main() {
    demo_state_t state;
    memset(&state, 0, sizeof(demo_state_t));

    // Initialize the camera.
    state.camera.x = 0.0f;
    state.camera.y = 0.0f;

    state.zoom.level = 1.0f;
    state.zoom.min_level = 0.1f;
    state.zoom.max_level = 10.0f;
    state.zoom.scroll_factor = 1.2f; // 1.0 scroll = 20% zoom

    // Set everything as dirty.
    state.is_dirty.any = 0xF;

    GLFWwindow *window = setup_glfw(W, H, &state);
    run(window);
    teardown_glfw(window);
    return EXIT_SUCCESS;
}