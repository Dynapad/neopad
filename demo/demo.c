#define DEMO_MODE_OPENGL 1
#define DEMO_MODE_BGFX 2

//#define DEMO_MODE DEMO_MODE_OPENGL
#define DEMO_MODE DEMO_MODE_BGFX

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#endif

#include <bgfx/c99/bgfx.h>

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>

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

// Shaders (OpenGL)
#include "util/gl_shaders.h"

// Shaders (BGFX)
#include <generated/shaders/demo/all.h>
#include "util/bgfx/c99_embedded_shader.h"

static const bgfx_embedded_shader_t embedded_shaders[] = {
        BGFX_EMBEDDED_SHADER(vs_basic),
        BGFX_EMBEDDED_SHADER(fs_basic),
        BGFX_EMBEDDED_SHADER_END()
};

// Utilities for BGFX<->GLFW

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

#include "cglm/mat4.h"
#include "cglm/cglm.h"

uint16_t uint16_max(uint16_t _a, uint16_t _b) {
    return _a < _b ? _b : _a;
}

#include <neopad/lib.h>

// May want to use GLAD / see: https://gen.glad.sh
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "common.h"


// XYZW coordinates for points of a unit quad (drawing as triangles).
float quad_coords[] = {-1.0f, 1.0f, 0.0f, 1.0f, // top left
                       -1.0f, -1.0f, 0.0f, 1.0f, // bottom left
                       1.0f, -1.0f, 0.0f, 1.0f, // bottom right
                       1.0f, -1.0f, 0.0f, 1.0f, // bottom right
                       1.0f, 1.0f, 0.0f, 1.0f,  // top right
                       -1.0f, 1.0f, 0.0f, 1.0f}; // top left

// RGBA colors for each point of the unit quad.
// top left: red
// top right: green
// bottom right: blue
// bottom left: white
float tex_data[] = {1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    1.0f, 1.0f, 1.0f, 1.0f};

float tex_coords[] = {0.0f, 0.0f, // top left -> red
                      1.0f, 0.0f, // bottom left -> green
                      0.0f, 1.0f,
                      0.0f, 1.0f,
                      1.0f, 1.0f,
                      0.0f, 0.0f};

void error_callback(int error, const char *description) {
    eprintf("GLFW Error: %s\n", description);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

float zoom = 1;

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    // If zoom is positive, zoom in. If zoom is negative, zoom out.
    printf("Scroll: %f\n", y_offset);
    // Limit zoom rate.
    zoom += glm_clamp((float) y_offset, -0.01f, 0.01f);
    // Clamp zoom between 0.1 and 1.
    zoom = glm_clamp(zoom, 0.1f, 1.0f);
}

GLFWwindow *demo_glfw_setup(int width, int height) {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        eprintf("Error: unable to initialize GLFW");
        exit(EXIT_FAILURE);
    }

#if DEMO_MODE == DEMO_MODE_OPENGL
    // Require OpenGL 3.3+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // Require a forward-compatible core profile (needed for macOS)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elif DEMO_MODE == DEMO_MODE_BGFX
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

    GLFWwindow *window = glfwCreateWindow(width, height, "Neopad Demo", NULL, NULL);
    if (!window) {
        eprintf("Error: unable to create window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);


#if DEMO_MODE == DEMO_MODE_OPENGL
    // Obtain a context (OpenGL only)
    glfwMakeContextCurrent(window);

    // Print some debug information.
    printf("OpenGL:   %s\n", glGetString(GL_VERSION));
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("GLFW:     %s\n", glfwGetVersionString());
#endif

    return window;;
}

void demo_glfw_teardown(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void demo_run_bgfx(GLFWwindow *window) {
    int res = false;
    int width, height;
    uint32_t debug = BGFX_DEBUG_TEXT;
    uint32_t reset = BGFX_RESET_VSYNC;
    glfwGetWindowSize(window, &width, &height);

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
    } else {
        printf("IT'S ALIVE!\n");
    }
    bgfx_reset(width, height, reset, init.resolution.format);

    // Enable debug text.
    bgfx_set_debug(debug);

    // Clear screen.
    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx_set_view_rect(0, 0, 0, width, height);



    while (!glfwWindowShouldClose(window)) {
        // Poll for events
        glfwPollEvents();

        // Handle window resize.
        glfwGetFramebufferSize(window, &width, &height);
        bgfx_reset(width, height, reset, init.resolution.format);
        bgfx_set_view_rect_ratio(0, 0, 0, BGFX_BACKBUFFER_RATIO_EQUAL);

        // This dummy draw call is here to make sure that view 0 is cleared
        // if no other draw calls are submitted to view 0.
        bgfx_encoder_t* encoder = bgfx_encoder_begin(true);
        bgfx_encoder_touch(encoder, 0);
        bgfx_encoder_end(encoder);

        // Use debug font to print information about this example.
        bgfx_dbg_text_clear(0, false);
        bgfx_dbg_text_printf(0, 0, 0x1f, __FILE__);



        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx_frame(false);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx_frame(false);
    }

    bgfx_shutdown();
}

void demo_run_opengl(GLFWwindow *window) {
    // VAO: Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO: Vertex Buffer Object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(quad_coords),
            &quad_coords[0],
            GL_STATIC_DRAW
    );
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TBO: Texture Buffer Object
    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(tex_coords),
            &tex_coords[0],
            GL_STATIC_DRAW
    );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create a texture, and set texture parameters (not strictly necessary)
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_FLOAT, tex_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Compile and use the shader program.
    GLuint program = create_shader_program(vert_shader_src, frag_shader_src);
    glUseProgram(program);

    // Extract IDs of shader variables.
    GLint mvp_loc = glGetUniformLocation(program, "mvp");
    assert(mvp_loc >= 0);
    if (mvp_loc < 0) {
        eprintf("Error: unable to find mvp uniform\n");
        eprintf("OpenGL Error: %x\n", glGetError());
        exit(EXIT_FAILURE);
    }

    GLint tex_loc = glGetUniformLocation(program, "tex");
    if (tex_loc < 0) {
        eprintf("Error: unable to find texture uniform\n");
        eprintf("OpenGL Error: %x\n", glGetError());
        exit(EXIT_FAILURE);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(tex_loc, 0);

    // Set a background color (dark blue)
    glClearColor(0.0f, 0.0f, 0.4f, 1.0f);

    // Run!
    while (!glfwWindowShouldClose(window)) {
        // Update the viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Update the projection matrix
        float aspect = (float) width / (float) height;

        // Update the model-view-projection matrix, applying the zoom level.

        mat4 proj_mat, model_view_mat, mvp_mat;
        glm_ortho_default(aspect, proj_mat);
        glm_mat4_identity(model_view_mat);
        glm_scale(model_view_mat, (vec3) {zoom, zoom, 1.0f});
        glm_mat4_mul(proj_mat, model_view_mat, mvp_mat);
        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, (GLfloat *) mvp_mat);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Select geometry
        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Draw the geometry
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Unbind
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindVertexArray(0);

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll for events
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &tbo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

int main() {
    int width = 1280;
    int height = 800;

    GLFWwindow *window = demo_glfw_setup(width, height);

#if DEMO_MODE == DEMO_MODE_OPENGL
    demo_run_opengl(window);
#elif DEMO_MODE == DEMO_MODE_BGFX
    demo_run_bgfx(window);
#endif

    demo_glfw_teardown(window);
    return EXIT_SUCCESS;
}