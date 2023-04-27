#include "demo.h"
#include <neopad/lib.h>

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

#pragma mark - Shaders
#include <generated/shaders/demo/all.h>
#include "util/bgfx/c99_embedded_shader.h"

static const bgfx_embedded_shader_t embedded_shaders[] = {
        BGFX_EMBEDDED_SHADER(vs_basic),
        BGFX_EMBEDDED_SHADER(fs_basic),
        BGFX_EMBEDDED_SHADER_END()
};

#pragma mark - Geometry
typedef struct demo_xy_rgb_s {
    vec2 xy;
    uint32_t rgba;
} demo_xy_rgb_t;

static const demo_xy_rgb_t demo_vertices[] = {
        {{-0.5f, -0.5f}, 0x339933FF},
        {{0.5f, -0.5f}, 0x993333FF},
        {{0.0f, 0.5f}, 0x333399FF},
};

static const uint16_t demo_indices[] = {
        0, 1, 2,
};

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

static void *demo_get_native_display_handle(GLFWwindow *window) {
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

#pragma mark - GLFW Callbacks
void error_callback(int error, const char *description) {
    eprintf("GLFW Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static float zoom = 1;
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    // If zoom is positive, zoom in. If zoom is negative, zoom out.
    printf("Scroll: %f\n", y_offset);
    // Limit zoom rate.
    zoom += glm_clamp((float) y_offset, -0.01f, 0.01f);
    // Clamp zoom between 0.1 and 1.
    zoom = glm_clamp(zoom, 0.1f, 1.0f);
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
    glfwSetScrollCallback(window, scroll_callback);

    return window;;
}

void teardown(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void run(GLFWwindow *window) {
    int res = false;
    int width, height;
    uint32_t debug = BGFX_DEBUG_STATS;
    uint32_t reset = BGFX_RESET_VSYNC;
    glfwGetFramebufferSize(window, &width, &height);

    // Prevent render thread creation (avoids a semaphore deadlock in init).
    bgfx_render_frame(0);

    // Initialize BGFX
    bgfx_init_t init;
    bgfx_init_ctor(&init);

    init.resolution.width = width;
    init.resolution.height = height;
    init.resolution.reset = reset;
    init.platformData = (bgfx_platform_data_t) {
            .nwh = demo_get_native_window_handle(window),
            .ndt = demo_get_native_display_handle(window)
    };

    res = bgfx_init(&init);
    if (!res) {
        eprintf("Error: unable to initialize BGFX\n");
        exit(EXIT_FAILURE);
    }

    bgfx_view_id_t view_id = 0;

    bgfx_reset(width, height, reset, init.resolution.format);

    // Enable debug text.
    bgfx_set_debug(debug);

    // Clear screen.
    bgfx_set_view_clear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx_set_view_rect(view_id, 0, 0, width, height);

    // Vertex layout.
    bgfx_vertex_layout_t vertex_layout;
    bgfx_vertex_layout_begin(&vertex_layout, BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&vertex_layout, BGFX_ATTRIB_POSITION, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_add(&vertex_layout, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true, false);
    bgfx_vertex_layout_end(&vertex_layout);

    // Create vertex buffer.
    bgfx_vertex_buffer_handle_t vertex_buffer_handle = bgfx_create_vertex_buffer(
            bgfx_make_ref(demo_vertices, sizeof(demo_vertices)), &vertex_layout, BGFX_BUFFER_NONE
    );
    bgfx_index_buffer_handle_t index_buffer_handle = bgfx_create_index_buffer(
            bgfx_make_ref(demo_indices, sizeof(demo_indices)), BGFX_BUFFER_NONE
    );

    // Compile shaders
    bgfx_renderer_type_t renderer_type = bgfx_get_renderer_type();
    bgfx_program_handle_t program = bgfx_create_program(
            bgfx_create_embedded_shader(embedded_shaders, renderer_type, "vs_basic"),
            bgfx_create_embedded_shader(embedded_shaders, renderer_type, "fs_basic"),
            true
    );

    while (!glfwWindowShouldClose(window)) {
        // Poll for events
        glfwPollEvents();

        // Handle window resize.
        glfwGetFramebufferSize(window, &width, &height);
        bgfx_reset(width, height, reset, init.resolution.format);
        bgfx_set_view_rect_ratio(view_id, 0, 0, BGFX_BACKBUFFER_RATIO_EQUAL);

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx_encoder_t* encoder = bgfx_encoder_begin(true);
        bgfx_encoder_touch(encoder, view_id);
        bgfx_encoder_end(encoder);

        // Use debug font to print information about this example.
        bgfx_dbg_text_clear(0, false);
        bgfx_dbg_text_printf(0, 0, 0x1f, __FILE__);

        // We just want to write RGB, default includes alpha, depth, depth testing, culling, etc.
        bgfx_set_state(BGFX_STATE_WRITE_RGB, 0);

        bgfx_set_vertex_buffer(0, vertex_buffer_handle, 0, 3);
        bgfx_set_index_buffer(index_buffer_handle, 0, 3);
        bgfx_submit(0, program, 0, false);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx_frame(false);
    }

    bgfx_shutdown();
}

int main() {
    int width = 1280;
    int height = 800;

    GLFWwindow *window = setup(width, height);

    run(window);

    teardown(window);
    return EXIT_SUCCESS;
}