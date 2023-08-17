//
// Created by Dylan Lukes on 4/28/23.
//

#ifndef NEOPAD_RENDERER_H
#define NEOPAD_RENDERER_H

#include <stdbool.h>
#include <stdint.h>

#include <neopad/types.h>

#pragma mark - Types

/// A renderer.
/// @note This is an opaque type.
typedef struct neopad_renderer_s *neopad_renderer_t;

/// A constant renderer.
/// @note For now this is more of a hint than a guarantee.
/// @todo Make this actually const. Currently blocked by cglm's lack of const qualifiers.
typedef /*const*/ struct neopad_renderer_s *neopad_renderer_const_t;

/// Initialization parameters for a renderer.
typedef struct neopad_renderer_init_s {
    /// The name of the renderer.
    const char *name;

    /// The width of the back-buffer(s) used by the renderer.
    int width;

    /// The height of the back-buffer(s) used by the renderer.
    int height;

    /// The scaling factor for the renderer. Used on high-DPI displays.
    float content_scale;

    /// Enable debug mode.
    bool debug;

    /// The native window handle.
    void *native_window_handle;

    /// The native display type.
    void *native_display_type;

    /// The background settings.
    struct {
        uint32_t color;
        bool grid_enabled;
        float grid_major;
        float grid_minor;
    } background;
} neopad_renderer_init_t;

#pragma mark - Lifecycle

/// Create (allocate) a renderer.
neopad_renderer_t neopad_renderer_create();

/// Destroy (deallocate) a renderer.
void neopad_renderer_destroy(neopad_renderer_t this);

/// Initialize the renderer. Call this on the thread that will be calling neopad_ functions.
void neopad_renderer_init(neopad_renderer_t this, neopad_renderer_init_t init);

/// Shutdown the renderer. Call this on the thread that called neopad_renderer_init().
void neopad_renderer_shutdown(neopad_renderer_t this);

#pragma mark - Frames

/// This is a blocking call, which will wait for the API thread to call neopad_renderer_end_frame.
/// @note Call BEFORE neopad_renderer_init on the thread that will be used for rendering. This may
///       be the same thread as the API thread, but it doesn't have to be.
/// @note Should ONLY be called on the render thread. On most systems, this MUST be the main thread.
/// @note See https://bkaradzic.github.io/bgfx/bgfx.html#_CPPv4N4bgfx11renderFrameE7int32_t
/// @param this The renderer.
/// @param timeout_ms A timeout in milliseconds to wait for a frame to finish.
void neopad_renderer_await_frame(neopad_renderer_t this, int timeout_ms);

/// Begin a frame.
/// @note This MUST be called on the API thread.
/// @param this The renderer.
void neopad_renderer_begin_frame(neopad_renderer_t this);

/// End a frame.
/// @param this The renderer.
/// @note This MUST be called on the API thread.
void neopad_renderer_end_frame(neopad_renderer_t this);

#pragma mark - Coordinate Transformations

/// Convert a point from window coordinates to world coordinates.
/// @param viewport The viewport of the window (left, top, right, bottom).
/// @param p The input point in window coordinates.
/// @param q The output point in world coordinates.
void neopad_renderer_window_to_world(neopad_renderer_const_t this, neopad_vec4_t viewport, neopad_vec2_t p, neopad_vec2_t *q);

/// Convert a point from screen coordinates to world coordinates.
/// @note This differs from the window-to-world transformation in that it takes the camera position into account.
///       This is important for the camera controls, where we want to avoid a feedback loop.
/// @param viewport The viewport of the renderer (left, top, right, bottom).
/// @param p The input point in window coordinates.
/// @param q The output point in screen coordinates.
void neopad_renderer_window_to_screen(neopad_renderer_const_t this, neopad_vec4_t viewport, neopad_vec2_t p, neopad_vec2_t *q);


#pragma mark - Manipulation

/// Resize the back-buffer(s) used by the renderer.
/// @note Takes effect from the start of the next begun frame.
void neopad_renderer_resize(neopad_renderer_t this, int width, int height);

/// Rescale the display of the renderer.
/// @note Takes effect on the next frame rendered.
void neopad_renderer_rescale(neopad_renderer_t this, float content_scale);

/// Set the zoom of the renderer.
/// @note Takes effect on the next frame rendered.
void neopad_renderer_zoom(neopad_renderer_t this, float zoom);

/// Arrest the zoom of the renderer.
/// @note This is useful for "canceling" a zoom gesture, for example.
/// @note Takes effect on the next frame rendered.
/// @return The current zoom.
float neopad_renderer_arrest_zoom(neopad_renderer_t this);

/// Reposition the viewport of the renderer.
/// @note In world coordinates.
/// @note Takes effect from the start of the next begun frame.
void neopad_renderer_get_camera(neopad_renderer_const_t this, neopad_vec2_t *dst);

/// Reposition the viewport of the renderer.
/// @note In world coordinates.
/// @note Takes effect from the start of the next begun frame.
void neopad_renderer_set_camera(neopad_renderer_t this, neopad_vec2_t src);

#pragma mark - Drawing

void neopad_renderer_draw_background(neopad_renderer_t this);
void neopad_renderer_draw_cursor(neopad_renderer_t this, vec2 p);
void neopad_renderer_draw_test_rect(neopad_renderer_t this, float l, float t, float r, float b);

#pragma mark - Line Drawing

/// Begin a series of points.
/// @param this The renderer.
/// @param p The first point.
void neopad_renderer_begin_points(neopad_renderer_t this, vec2 p);

/// Add a point to the current series.
/// @param this The renderer.
/// @param p The point to add.
void neopad_renderer_pen_add_point(neopad_renderer_t this, vec2 p);

/// End a series of points.
/// @param this The renderer.
void neopad_renderer_end_points(neopad_renderer_t this);

#pragma mark - Demo

#endif //NEOPAD_RENDERER_H
