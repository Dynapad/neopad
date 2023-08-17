//
// Created by Dylan Lukes on 8/16/23.
//

#ifndef NEOPAD_DEMO_H
#define NEOPAD_DEMO_H

#include "platform.h"
#include <neopad/renderer.h>

/** GLFW callbacks: demo_input.c */
void error_callback(int error, const char *description);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void content_scale_callback(GLFWwindow *window, float xscale, float yscale);

/** Our own functions: demo.c */
void update(GLFWwindow *window);
void draw(GLFWwindow *window);

/** Demo app state. Everything goes in here. */
typedef struct {
    neopad_renderer_t renderer;

    // Window size and content scale.
    ivec2 size;
    float content_scale;

    /// Flags for what needs to be updated next loop.
    bool is_size_dirty; // have we resized to match yet?
    bool is_content_scale_dirty;

    union {
        struct {
            bool size: 1;
            bool content_scale: 1;
            bool fullscreen;
            bool camera: 1;
            bool zoom: 1;
        };
        uint8_t any;
    } is_dirty;

    /// Last known camera position.
    vec2 camera;

    /// Last known cursor details.
    struct {
        vec2 pos;
        bool is_down;
    } cursor;

    struct {
        /// Are we in fullscreen mode?
        bool is_active;

        /// Last known windowed position and size (e.g. to return to after fullscreen)
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

#endif //NEOPAD_DEMO_H
