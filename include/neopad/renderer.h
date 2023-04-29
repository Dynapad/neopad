//
// Created by Dylan Lukes on 4/28/23.
//

#ifndef NEOPAD_RENDERER_H
#define NEOPAD_RENDERER_H

#include <stdbool.h>
#include <stdint.h>

#pragma mark - Types

/// A renderer.
/// \note This is an opaque type.
typedef struct neopad_renderer_s *neopad_renderer_t;

/// Initialization parameters for a renderer.
typedef struct neopad_renderer_init_s {
    const char *name;
    int width;
    int height;
    bool debug;

    void *native_window_handle;
    void *native_display_type;
} neopad_renderer_init_t;

#pragma mark - Lifecycle

/// Create a renderer.
neopad_renderer_t neopad_renderer_create();

/// Initialize a renderer.
/// \param init The initialization parameters.
void neopad_renderer_init(neopad_renderer_t this, neopad_renderer_init_t init);

/// Resize the back-buffer(s) used by the renderer.
/// \note The effect takes place at the start of the next frame.
void neopad_renderer_resize(neopad_renderer_t this, int width, int height);

/// Destroy a renderer.
void neopad_renderer_destroy(neopad_renderer_t this);

#pragma mark - Frame

/// Begin a frame.
/// \param this The renderer.
void neopad_renderer_begin_frame(neopad_renderer_t this);

/// End a frame.
/// \param this The renderer.
void neopad_renderer_end_frame(neopad_renderer_t this);

#pragma mark - Drawing

/// Clear the screen.
/// \param this The renderer.
/// \param color The color to clear the screen to.
void neopad_renderer_clear(neopad_renderer_t this, uint32_t color);

void neopad_renderer_test_rect(neopad_renderer_t this);

#pragma mark - Line Drawing

void neopad_renderer_pen_begin(neopad_renderer_t this, float x, float y);

void neopad_renderer_pen_draw_to(neopad_renderer_t this, float x, float y);

void neopad_renderer_pen_end(neopad_renderer_t this, float x, float y);


#pragma mark - Demo

#endif //NEOPAD_RENDERER_H
