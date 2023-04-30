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

#pragma mark - Forward Declarations

void draw(GLFWwindow *window);

#pragma mark - Globals

static neopad_renderer_t renderer = NULL;

#pragma mark - GLFW Callbacks

void error_callback(int error, const char *description) {
    eprintf("GLFW Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static bool mouse_down = false;
static double drag_start_x = 0;
static double drag_start_y = 0;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouse_down = true;
            glfwGetCursorPos(window, &drag_start_x, &drag_start_y);
        } else if (action == GLFW_RELEASE) {
            mouse_down = false;
        }
    }
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (mouse_down) {
        float delta_x = x - drag_start_x;
        float delta_y = -(y - drag_start_y);
        printf("Drag delta: %f, %f\n", delta_x, delta_y);

        neopad_renderer_set_position(renderer, delta_x, delta_y);
    }
}

static float zoom = 1;
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    // If zoom is positive, zoom in. If zoom is negative, zoom out.

    // Limit zoom rate.
    zoom += glm_clamp((float) y_offset, -0.01f, 0.01f);
    // Clamp zoom betsween 0.1 and 2.0.
    zoom = glm_clamp(zoom, 0.1f, 2.0f);

    neopad_renderer_zoom(renderer, zoom);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (renderer == NULL) return;
    neopad_renderer_resize(renderer, width, height);
    draw(window);
}

void window_content_scale_callback(GLFWwindow* window, float xscale, float yscale)
{
    if (renderer == NULL) return;
    printf("Content scale: %f, %f\n", xscale, yscale);
    neopad_renderer_rescale(renderer, xscale);
    draw(window);
}

#pragma mark - Setup / Teardown

GLFWwindow *setup(int width, int height) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        eprintf("Error: unable to initialize GLFW");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Neopad Demo", NULL, NULL);
    if (!window) {
        eprintf("Error: unable to create window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

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
    neopad_renderer_draw_test_rect(renderer, -200, -100, -100, -200);
    neopad_renderer_draw_test_rect(renderer, 400, 300, 500, 200);
    neopad_renderer_end_frame(renderer);
}

void run(GLFWwindow *window) {
    int res = false;
    int width, height;
    float scale = 1.0f;

    glfwGetFramebufferSize(window, &width, &height);
    glfwGetWindowContentScale(window, &scale, NULL);

    renderer = neopad_renderer_create();
    neopad_renderer_init(renderer, (neopad_renderer_init_t) {
        .width = width,
        .height = height,
        .content_scale = scale,
        .debug = true,
        .native_window_handle = demo_get_native_window_handle(window),
        .native_display_type = demo_get_native_display_type(window),
        .background = (neopad_renderer_background_t) {
            .color = 0x202020FF,
            .grid_enabled = true,
            .grid_major = 100,
            .grid_minor = 25,
        },
    });

    while (!glfwWindowShouldClose(window)) {
        // Poll for events and check if the window resized.
        glfwPollEvents();
        draw(window);
    }

    neopad_renderer_destroy(renderer);
}

int main() {
    int width = 1300;
    int height = 800;

    GLFWwindow *window = setup(width, height);

    run(window);

    teardown(window);
    return EXIT_SUCCESS;
}