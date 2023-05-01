//
// Created by Dylan Lukes on 4/28/23.
//

#ifndef NEOPAD_RENDERER_H
#define NEOPAD_RENDERER_H

#include <stdbool.h>
#include <stdint.h>
#include "cglm/vec2.h"

#pragma mark - Types

/// A renderer.
/// @note This is an opaque type.
typedef struct neopad_renderer_s *neopad_renderer_t;

/// A constant renderer.
/// @note For now this is more of a hint than a guarantee.
/// @todo Make this actually const. Currently blocked by cglm's lack of const qualifiers.
typedef /*const*/ struct neopad_renderer_s *neopad_renderer_const_t;

typedef struct neopad_renderer_background_s {
    uint32_t color;

    bool grid_enabled;
    float grid_major;
    float grid_minor;

    uint32_t x_axis_color;
    uint32_t y_axis_color;
} neopad_renderer_background_t;

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
    neopad_renderer_background_t background;
} neopad_renderer_init_t;

#pragma mark - Lifecycle

/// Create a renderer.
neopad_renderer_t neopad_renderer_create();

/// Initialize a renderer.
/// \param init The initialization parameters.
void neopad_renderer_init(neopad_renderer_t this, neopad_renderer_init_t init);

/// Destroy a renderer.
void neopad_renderer_destroy(neopad_renderer_t this);

#pragma mark - Coordinate Transformations

/// Convert a point from window coordinates to world coordinates.
/// @param viewport The viewport of the window (left, top, right, bottom).
/// @param p The input point in window coordinates.
/// @param q The output point in world coordinates.
void neopad_renderer_window_to_world(neopad_renderer_const_t this, const vec4 viewport, const vec2 p, vec2 q);

/// Convert a point from screen coordinates to world coordinates.
/// @note This differs from the window-to-world transformation in that it takes the camera position into account.
///       This is important for the camera controls, where we want to avoid a feedback loop.
/// @param viewport The viewport of the renderer (left, top, right, bottom).
/// @param p The input point in window coordinates.
/// @param q The output point in screen coordinates.
void neopad_renderer_window_to_screen(neopad_renderer_const_t this, const vec4 viewport, const vec2 p, vec2 q);


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

/// Reposition the viewport of the renderer.
/// @note In world coordinates.
/// @note Takes effect from the start of the next begun frame.
void neopad_renderer_get_camera(neopad_renderer_const_t this, vec2 dest);

/// Reposition the viewport of the renderer.
/// @note In world coordinates.
/// @note Takes effect from the start of the next begun frame.
void neopad_renderer_set_camera(neopad_renderer_t this, const vec2 src);

#pragma mark - Frame

/// Begin a frame.
/// \param this The renderer.
void neopad_renderer_begin_frame(neopad_renderer_t this);

/// End a frame.
/// \param this The renderer.
void neopad_renderer_end_frame(neopad_renderer_t this);

#pragma mark - Drawing

void neopad_renderer_draw_background(neopad_renderer_t this);

void neopad_renderer_draw_test_rect(neopad_renderer_t this, float l, float t, float r, float b);

#pragma mark - Line Drawing

void neopad_renderer_pen_begin(neopad_renderer_t this, float x, float y);

void neopad_renderer_pen_draw_to(neopad_renderer_t this, float x, float y);

void neopad_renderer_pen_end(neopad_renderer_t this, float x, float y);


#pragma mark - Demo

#endif //NEOPAD_RENDERER_H
