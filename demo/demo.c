#include <neopad/renderer.h>
#include <neopad/neopad.h>

#include <stdio.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#pragma mark - Includes
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

#include <bx/platform.h>

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if ENTRY_CONFIG_USE_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	else
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#	endif
#elif BX_PLATFORM_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif

#include <GLFW/glfw3native.h>

#include <cglm/vec2.h>
#include "neopad/types.h"

#pragma mark - Native Handles

static void *demo_get_native_window_handle(GLFWwindow *window) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
    wl_egl_window *win_impl = (wl_egl_window*)glfwGetWindowUserPointer(window);
    if(!win_impl)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        struct wl_surface* surface = (struct wl_surface*)glfwGetWaylandWindow(window);
        if(!surface)
            return nullptr;
        win_impl = wl_egl_window_create(surface, width, height);
        glfwSetWindowUserPointer(window, (void*)(uintptr_t)win_impl);
    }
    return (void*)(uintptr_t)win_impl;
#else
    return (void*)(uintptr_t)glfwGetX11Window(window);
#endif
#elif BX_PLATFORM_OSX
    return glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
    return glfwGetWin32Window(window);
#endif
}

static void *demo_get_native_display_type(GLFWwindow *window) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
    return glfwGetWaylandDisplay();
#else
    return glfwGetX11Display();
#endif
#else
    return NULL;
#endif
}

static void *demo_get_native_context(GLFWwindow *window) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
    return glfwGetWaylandContext(window);
#else
    return glfwGetGLXContext(window);
#endif
#elif BX_PLATFORM_OSX
    return glfwGetNSGLContext(window);
#elif BX_PLATFORM_WINDOWS
    return glfwGetWGLContext(window);
#else
    return NULL;
#endif
}

#include "util.h"

#pragma mark - Forward Declarations

void draw(GLFWwindow *window);

#pragma mark - Globals

static neopad_renderer_t renderer = NULL;

#pragma mark - Demo State

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
        vec2 initial_camera;
    } drag;

    struct {
        /// Is the user zooming?
        bool is_zooming;

        /// Current zoom level.
        float level;

        // Maximum and minimum (config).
        float max_level;
        float min_level;
    } zoom;
} demo_state_t;

void toggle_fullscreen(GLFWwindow *window) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // If we're already in fullscreen mode, exit it.
    if (state->fullscreen.is_active) {
        int *pos = &state->fullscreen.saved_window.pos[0];
        int *size = &state->fullscreen.saved_window.size[0];

        neopad_renderer_resize(renderer, size[0], size[1]);
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
    neopad_renderer_resize(renderer, mode->width, mode->height);

    // Switch to fullscreen mode.
    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    state->fullscreen.is_active = true;
}

// Zooming
const float ZOOM_FACTOR = 1.2f;
static float zoom = 1.0f;

// Dragging
static bool mouse_down = false;
static vec2 drag_from = {0, 0};
static vec2 drag_to = {0, 0};
static vec2 camera_start = {0, 0};

#pragma mark - Utility Functions

void get_cursor_pos(GLFWwindow *window, vec2 dest) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    dest[0] = (float) x, dest[1] = (float) y;
}

void get_viewport(GLFWwindow *window, vec4 dest) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    dest[0] = 0, dest[1] = 0, dest[2] = (float) w, dest[3] = (float) h;
}

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
                neopad_renderer_set_camera(renderer, (vec2) {0, 0});
                neopad_renderer_zoom(renderer, 1);
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
                // 9, 8, 7, 6, 5, 4, 3, 2, 1, Shift-2, Shift-3, Shift-4, Shift-5, etc
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

                neopad_renderer_set_camera(renderer, (vec2) {0, 0});
                neopad_renderer_zoom(renderer, zoom);

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
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        neopad_renderer_arrest_zoom(renderer);
        if (action == GLFW_PRESS) {
            mouse_down = true;
            get_cursor_pos(window, drag_from);

            vec4 viewport;
            get_viewport(window, viewport);
            neopad_renderer_window_to_screen(renderer, viewport, drag_from, drag_from);

            // Save the starting camera position.
            neopad_renderer_get_camera(renderer, camera_start);
        } else if (action == GLFW_RELEASE) {
            mouse_down = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (mouse_down) {
        get_cursor_pos(window, drag_to);

        vec4 viewport;
        get_viewport(window, viewport);
        neopad_renderer_window_to_screen(renderer, viewport, drag_to, drag_to);

        vec2 drag_delta;
        glm_vec2_sub(drag_to, drag_from, drag_delta);

        vec2 camera;
        glm_vec2_add(camera_start, drag_delta, camera);
        neopad_renderer_set_camera(renderer, camera);
    }
}

static double dead_zone = 5.0;
static double last_y_offset = 0.0;

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    demo_state_t *state = (demo_state_t *) glfwGetWindowUserPointer(window);

    // If the direction of the scroll changed, arrest any ongoing zoom,
    // even if it's in the dead zone.
    bool dir_changed = last_y_offset * y_offset < 0;
    bool in_dead_zone = fabs(y_offset) < dead_zone;

//    if (dir_changed) {
//        printf("dir changed\n");
//        state->zoom.level = neopad_renderer_arrest_zoom(renderer);
//        last_y_offset = y_offset;
//    }
//
//    // If we're in the dead zone, do nothing.
//    // The dead zone is 3x as large if we just arrested the zoom.
//    if (fabs(y_offset) < dead_zone * (3 * dir_changed)) {
//        printf("dead zone %f\n", fabs(y_offset));
//        return;
//    }

    // Adjust the zoom level logarithmically.
    if (y_offset > 0) zoom *= powf(ZOOM_FACTOR, (float) y_offset);
    else if (y_offset < 0) zoom /= powf(ZOOM_FACTOR, (float) -y_offset);

    // Clamp zoom betwixt 0.1 and 10.0.
    zoom = glm_clamp(zoom, state->zoom.min_level, state->zoom.max_level);
    neopad_renderer_zoom(renderer, zoom);

    last_y_offset = y_offset;
}

void window_size_callback(GLFWwindow *window, int width, int height) {

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    if (renderer == NULL) return;
    neopad_renderer_resize(renderer, width, height);
    draw(window);
}

void window_content_scale_callback(GLFWwindow *window, float xscale, float yscale) {
    if (renderer == NULL) return;
    printf("Content scale: %f, %f\n", xscale, yscale);
    neopad_renderer_rescale(renderer, xscale);
    draw(window);
}

#pragma mark - Setup / Teardown

GLFWwindow *setup(int width, int height, demo_state_t *state) {
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
    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
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

void teardown(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void draw(GLFWwindow *window) {
    neopad_renderer_begin_frame(renderer);
    neopad_renderer_draw_background(renderer);
    neopad_renderer_draw_test_rect(renderer, -100, 100, 100, -100);
    neopad_renderer_end_frame(renderer);
}

void run(GLFWwindow *window) {
    int width, height;
    float scale = 1.0f;

    glfwGetFramebufferSize(window, &width, &height);
    glfwGetWindowContentScale(window, &scale, NULL);

    neopad_renderer_init_t init = {
            .width = width,
            .height = height,
            .content_scale = scale,
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

    renderer = neopad_renderer_create();
    neopad_renderer_await_frame(renderer, 0); // marks this as the render thread
    neopad_renderer_init(renderer, init);  // marks this as the API thread

    while (!glfwWindowShouldClose(window)) {
        draw(window);
        glfwPollEvents();
    }

    neopad_renderer_shutdown(renderer);
    neopad_renderer_destroy(renderer);
}

int main() {
    demo_state_t state;
    state.zoom.min_level = 0.1f;
    state.zoom.max_level = 10.0f;

    GLFWwindow *window = setup(1200, 800, &state);
    run(window);
    teardown(window);
    return EXIT_SUCCESS;
}