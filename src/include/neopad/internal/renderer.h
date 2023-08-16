//
// Created by Dylan Lukes on 5/1/23.
//
// This is the internal header for the renderer core. Implementation files can include this
// header to access the contents of the renderer opaque pointer type. It should not be
// exposed in the public API.

#ifndef NEOPAD_RENDERER_INTERNAL_H
#define NEOPAD_RENDERER_INTERNAL_H

#include <cglm/vec2.h>

#include "neopad/internal/util/bgfx/embedded_shader.h"
#include "generated/shaders/src/all.h"

#include "neopad/types.h"
#include "neopad/renderer.h"
#include "neopad/internal/renderer/module.h"
#include "neopad/internal/util/bx/thread.h"

// todo: these defines could collide?

#define PROGRAM_BASIC 0
#define PROGRAM_BACKGROUND 1
#define PROGRAM_COUNT 2

#define VIEW_BACKGROUND 0
#define VIEW_CONTENT 1

/// @note This is a pad-world coordinate.
/// @note At zoom 1.0, these coordinates map to logical pixels.
/// @note However, (0, 0) is the center of the screen.
typedef struct __attribute__ ((__packed__)) {
    vec4 xyzw;
    uint32_t argb;
} neopad_renderer_vertex_t;

/// Structure matching defines in shaders/uniforms.sh
typedef struct {
    float time;
    float zoom;
    float _unused1;
    float _unused2;

    float _unused3;
    float _unused4;
    float grid_major;
    float grid_minor;
} neopad_renderer_uniforms_t;

struct neopad_renderer_s {
    /// Our own initialization parameters.
    neopad_renderer_init_t init;

    /// BGFX initialization parameters.
    bgfx_init_t bgfx_init;

    /// The rendering thread.
    bx_thread_t render_thread;

    /// Back-buffer resolution.
    /// @note For high-DPI displays, this is the resolution before scaling down.
    /// @note Logical width is width / content_scale.
    uint32_t width;
    uint32_t height;

    /// Intended back-buffer resolution post-resize.
    uint32_t target_width;
    uint32_t target_height;

    /// Content scale. This is used to scale the UI on high-DPI displays.
    /// @note A value of 1 means no scaling.
    /// @note A value of 2 means a 2:1 ratio from logical pixel size to display pixel size (hi-dpi).
    float content_scale;

    /// Offset. This controls the position of the camera.
    /// @note A value of 0 means the content is centered at (0, 0).
    /// @note In pad-world coordinates.
    vec2 camera;
    vec2 target_camera;

    /// Zoom. This controls how much of the content is visible.
    /// @note - A value of 0.5 means the content is displayed at 50%.
    /// @note - A value of 2.0 means the content is displayed at 200%.
    float zoom;
    float target_zoom;

    /// Matrices.
    /// @note These are kept around to facilitate things like dragging,
    ///       which requires being able to translate between screen and
    ///       pad-world coordinates.
    mat4 model;
    mat4 view;
    mat4 model_view;
    mat4 proj;

    /// Vertex layout(s).
    bgfx_vertex_layout_t vertex_layout;

    /// Programs (shader pipelines)
    bgfx_program_handle_t programs[PROGRAM_COUNT];

    /// Uniforms
    neopad_renderer_uniforms_t uniforms;
    bgfx_uniform_handle_t uniform_handle;

    /// Modules
    /// @todo Fix this (temporary hack)
    neopad_renderer_module_t modules[NEOPAD_RENDERER_MODULE_COUNT];
};


static const bgfx_embedded_shader_t embedded_shaders[] = {
        BGFX_EMBEDDED_SHADER(vs_basic),
        BGFX_EMBEDDED_SHADER(fs_basic),

        BGFX_EMBEDDED_SHADER(vs_grid),
        BGFX_EMBEDDED_SHADER(fs_grid),
        BGFX_EMBEDDED_SHADER_END()
};

#endif //NEOPAD_RENDERER_INTERNAL_H
