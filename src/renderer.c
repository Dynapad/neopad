//
// Created by Dylan Lukes on 4/28/23.
//
#include "neopad/renderer.h"
#include "util/log.h"
#include "util/bgfx/embedded_shader.h"
#include "generated/shaders/src/all.h"
#include "cglm/affine.h"

#include <cglm/vec3.h>
#include <cglm/cam.h>

#include <memory.h>

#define PROGRAM_BASIC 0
#define PROGRAM_BACKGROUND 1
#define PROGRAM_COUNT 2

#define VIEW_BACKGROUND 0
#define VIEW_CONTENT 1

/// @note This is a pad-world coordinate.
/// @note At zoom 1.0, these coordinates map to logical pixels.
/// @note However, (0, 0) is the center of the screen.
typedef struct neopad_vertex_s {
    float x;
    float y;
    float z;
    uint32_t argb;
} neopad_renderer_vertex_t;

/// Structure matching defines in shaders/uniforms.sh
typedef struct neopad_uniforms_s {
    float time;
    float content_scale;
    float zoom;
    float _unused;

    float offset_x;
    float offset_y;
    float grid_major;
    float grid_minor;
} neopad_renderer_uniforms_t;

struct neopad_renderer_s {
    /// BGFX initialization parameters.
    bgfx_init_t init;

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
    float target_content_scale;

    /// Offset. This controls the position of the camera.
    /// @note A value of 0 means the content is centered at (0, 0).
    /// @note In pad-world coordinates.
    float camera_x;
    float camera_y;
    float target_camera_x;
    float target_camera_y;

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

    /// Background settings.
    neopad_renderer_background_t background;

    /// Vertex layout(s).
    bgfx_vertex_layout_t vertex_layout;

    /// Programs (shader pipelines)
    bgfx_program_handle_t programs[PROGRAM_COUNT];

    /// Uniforms
    neopad_renderer_uniforms_t uniforms;
    bgfx_uniform_handle_t uniform_handle;

    // TODO: remove
    bgfx_dynamic_vertex_buffer_handle_t vertex_buffer;
    bgfx_dynamic_index_buffer_handle_t index_buffer;
};


static const bgfx_embedded_shader_t embedded_shaders[] = {
        BGFX_EMBEDDED_SHADER(vs_basic),
        BGFX_EMBEDDED_SHADER(fs_basic),

        BGFX_EMBEDDED_SHADER(vs_grid),
        BGFX_EMBEDDED_SHADER(fs_grid),
        BGFX_EMBEDDED_SHADER_END()
};

#pragma mark - Lifecycle

neopad_renderer_t neopad_renderer_create() {
    neopad_renderer_t renderer = malloc(sizeof(struct neopad_renderer_s));
    memset(renderer, 0, sizeof(struct neopad_renderer_s));
    return renderer;
}

void neopad_renderer_init(neopad_renderer_t this, neopad_renderer_init_t init) {
    // Populate ourselves
    this->width = this->target_width = init.width;
    this->height = this->target_height = init.height;
    this->camera_x = this->target_camera_x = 0.0f;
    this->camera_y = this->target_camera_y = 0.0f;
    this->content_scale = this->target_content_scale = init.content_scale > 0 ? init.content_scale : 1.0f;
    this->zoom = this->target_zoom = 1.0f;

    this->background = init.background;

    // Switch to single-threaded mode for simplicity...
    // See: https://bkaradzic.github.io/bgfx/internals.html
    bgfx_render_frame(0);

    // Initialize BGFX
    bgfx_init_ctor(&this->init);
    this->init.resolution.width = init.width;
    this->init.resolution.height = init.height;
    this->init.platformData.nwh = init.native_window_handle;
    this->init.platformData.ndt = init.native_display_type;

    if (!bgfx_init(&this->init)) {
        eprintf("Failed to initialize BGFX.\n");
        exit(EXIT_FAILURE);
    }

    bgfx_set_debug(init.debug ? BGFX_DEBUG_TEXT : 0);

    // Initialize vertex layout
    bgfx_vertex_layout_begin(&this->vertex_layout, BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&this->vertex_layout, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_add(&this->vertex_layout, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true, false);
    bgfx_vertex_layout_end(&this->vertex_layout);

    // Initialize programs
    bgfx_renderer_type_t renderer_type = bgfx_get_renderer_type();
    this->programs[PROGRAM_BASIC] = bgfx_create_embedded_program(
            embedded_shaders,
            renderer_type,
            "vs_basic",
            "fs_basic",
            NULL);

    this->programs[PROGRAM_BACKGROUND] = bgfx_create_embedded_program(
            embedded_shaders,
            renderer_type,
            "vs_grid",
            "fs_grid",
            NULL);

    // Initialize uniforms
    this->uniforms = (neopad_renderer_uniforms_t) {
            .time = 0.0f,
            .content_scale = this->content_scale,
            .zoom = this->zoom,
            .offset_x = this->camera_x,
            .offset_y = this->camera_y,
            .grid_major = this->background.grid_major,
            .grid_minor = this->background.grid_minor
    };
    this->uniform_handle = bgfx_create_uniform("u_params", BGFX_UNIFORM_TYPE_VEC4, 2);
    bgfx_set_uniform(this->uniform_handle, &this->uniforms, 2);

    // TODO: REMOVE THIS TEST STUFF
    // Initialize test vertex buffer
    this->vertex_buffer = bgfx_create_dynamic_vertex_buffer(
            4,
            &this->vertex_layout,
            BGFX_BUFFER_NONE);

    // Initialize test index buffer
    this->index_buffer = bgfx_create_dynamic_index_buffer(
            6,
            BGFX_BUFFER_NONE);
}

void neopad_renderer_destroy(neopad_renderer_t this) {
    bgfx_destroy_dynamic_index_buffer(this->index_buffer);
    bgfx_destroy_dynamic_vertex_buffer(this->vertex_buffer);
    bgfx_destroy_uniform(this->uniform_handle);
    bgfx_destroy_program(this->programs[PROGRAM_BACKGROUND]);
    bgfx_destroy_program(this->programs[PROGRAM_BASIC]);
    bgfx_shutdown();
    free(this);
}

#pragma mark - Coordinate Transformations

void neopad_renderer_screen2world(neopad_renderer_const_t this, const vec4 viewport, const vec2 window, vec2 world) {
    // Create the transforms we need.
    mat4 viewport_to_ndc;
    mat4 inv_model_view;
    mat4 inv_proj;
    glm_ortho(viewport[0], viewport[2], viewport[3], viewport[1], -1.0f, 1.0f, viewport_to_ndc);

    glm_mat4_inv(this->view, inv_model_view);
    glm_mat4_inv(this->proj, inv_proj);

    vec4 v = {window[0], window[1], 0.0f, 1.0f};
    vec4 w;

    glm_mat4_mulv(viewport_to_ndc, v, w);
    glm_mat4_mulv(inv_proj, w, w);
    glm_mat4_mulv(inv_model_view, w, w);

    glm_vec2_copy((vec2) {w[0], w[1]}, world);
}

#pragma mark - Manipualtion

void neopad_renderer_resize(neopad_renderer_t this, int width, int height) {
    this->target_width = width;
    this->target_height = height;
}

void neopad_renderer_rescale(neopad_renderer_t this, float content_scale) {
    this->target_content_scale = content_scale;
}

void neopad_renderer_zoom(neopad_renderer_t this, float zoom) {
    this->target_zoom = zoom;
}

void neopad_renderer_get_camera(neopad_renderer_const_t this, vec2 dst) {
    dst[0] = this->target_camera_x;
    dst[1] = this->target_camera_y;
}

void neopad_renderer_set_camera(neopad_renderer_t this, const vec2 src) {
    this->target_camera_x = src[0];
    this->target_camera_y = src[1];
}

#pragma mark - Rendering

void neopad_renderer_begin_frame(neopad_renderer_t this) {
    if (this->width != this->target_width || this->height != this->target_height) {
        this->width = this->target_width;
        this->height = this->target_height;
        bgfx_reset(this->width, this->height, BGFX_RESET_VSYNC, this->init.resolution.format);
    }

    if (this->camera_x != this->target_camera_x || this->camera_y != this->target_camera_y) {
        this->uniforms.offset_x = this->camera_x = this->target_camera_x;
        this->uniforms.offset_y = this->camera_y = this->target_camera_y;
        bgfx_set_uniform(this->uniform_handle, &this->uniforms, 2);
    }

    if (this->content_scale != this->target_content_scale) {
        this->uniforms.content_scale = this->content_scale = this->target_content_scale;
        bgfx_set_uniform(this->uniform_handle, &this->uniforms, 2);
    }

    if (this->zoom != this->target_zoom) {
        this->uniforms.zoom = this->zoom = this->target_zoom;
        bgfx_set_uniform(this->uniform_handle, &this->uniforms, 2);
    }
}

void neopad_renderer_end_frame(neopad_renderer_t this) {

    float width = (float) this->width;
    float height = (float) this->height;

    float scaled_width = (float) this->width / this->content_scale;
    float scaled_height = (float) this->height / this->content_scale;

    vec3 camera = {this->camera_x, this->camera_y, 0.0f};
    float zoom = this->zoom;

    // MATRICES
    // --------

    // Model matrix applies the content_scale to x and y (leaves z and w alone).
    // - On non-retina displays, this is an identity matrix.
    // - On retina displays, this will make the content appear at the same size as on non-retina displays.
    glm_mat4_identity(this->model);
    glm_scale(this->model, (vec3) {this->content_scale, this->content_scale, 1.0f});

    // View matrix applies camera (camera position).
    vec3 eye = {0.0f, 0.0f, 1.0f};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    glm_vec3_sub(eye, camera, eye);
    glm_vec3_sub(center, camera, center);
    glm_lookat(eye, center, up, this->view);

    // Premultiply the model and view matrices.
    glm_mat4_mul(this->model, this->view, this->model_view);

    // Orthographic Projection - View -> NDC
    // (-w/2, -w/2), (t/2, -t/2) -> (-1, -1), (1, 1)
    // Note that width/height here include the content scale.
    float l = -width / 2.0f;
    float r = width / 2.0f;
    float b = -height / 2.0f;
    float t = height / 2.0f;
    glm_ortho(l / zoom, r / zoom, b / zoom, t / zoom, -1.0f, 1.0f, this->proj);

    // BACKGROUND
    // ----------
    // The background uses the same view and projection matrices as the content, but
    // it does not apply them the same way. The background is always drawn at the same
    // size, regardless of the content scale or zoom. It renders geometry provided in
    // clip-space coordinates. It needs the same view and projection matrices so it can
    // invert them and use them to transform the clip-space coordinates into world-space
    // coordinates. This allows the background to be drawn in the same world-space as the
    // content, but at a fixed size.

    bgfx_set_view_clear(VIEW_BACKGROUND, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, this->background.color, 1.0f, 0);
    bgfx_set_view_transform(VIEW_BACKGROUND, this->model_view, this->proj);
    bgfx_set_view_rect(VIEW_BACKGROUND, 0, 0, this->width, this->height);
    bgfx_touch(VIEW_BACKGROUND);

    // CONTENT
    // -------

    bgfx_set_view_transform(VIEW_CONTENT, this->model_view, this->proj);
    bgfx_set_view_rect(VIEW_CONTENT, 0, 0, this->width, this->height);
    bgfx_touch(VIEW_CONTENT);

    // DEBUG
    // -----

    // Display current camera coordinates, zoom, content-scale.
    bgfx_dbg_text_clear(0, false);
    bgfx_dbg_text_printf(0, 0, 0x0f, "Camera: %f, %f", this->camera_x, this->camera_y);
    bgfx_dbg_text_printf(0, 1, 0x0f, "  Zoom: %f", this->zoom);
    bgfx_dbg_text_printf(0, 2, 0x0f, " Scale: %f", this->content_scale);

    bgfx_frame(false);
}

#pragma mark - Drawing

void neopad_renderer_draw_background(neopad_renderer_t this) {
    if (!this->background.grid_enabled) {
        return;
    }

    bgfx_transient_vertex_buffer_t tvb;
    bgfx_transient_index_buffer_t tib;

    bgfx_alloc_transient_vertex_buffer(&tvb, 4, &this->vertex_layout);
    bgfx_alloc_transient_index_buffer(&tib, 6, false);

    // NDC full quad.
    float l = -1.0f;
    float r = 1.0f;
    float t = -1.0f;
    float b = 1.0f;

    mat2 ndx_rect = {
            {l, t},
            {r, b},
    };

    neopad_renderer_vertex_t vertices[] = {
            {l, t, 0.0f, 0x00000000},
            {r, t, 0.0f, 0x00000000},
            {r, b, 0.0f, 0x00000000},
            {l, b, 0.0f, 0x00000000},
    };
    memcpy(tvb.data, vertices, sizeof(vertices));

    uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3,
    };
    memcpy(tib.data, indices, sizeof(indices));

    bgfx_set_transient_vertex_buffer(0, &tvb, 0, 4);
    bgfx_set_transient_index_buffer(&tib, 0, 6);
    bgfx_set_view_clear(VIEW_BACKGROUND, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, this->background.color, 1.0f, 0);

    bgfx_set_state(BGFX_STATE_WRITE_RGB
                   | BGFX_STATE_WRITE_A
                   | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_DST_ALPHA)
                   | BGFX_STATE_BLEND_EQUATION_SEPARATE(BGFX_STATE_BLEND_EQUATION_ADD, BGFX_STATE_BLEND_EQUATION_MAX),
                   0);
    bgfx_submit(VIEW_BACKGROUND, this->programs[PROGRAM_BACKGROUND], 0, false);
}

#pragma mark - Testing

void neopad_renderer_draw_test_rect(neopad_renderer_t this, float l, float t, float r, float b) {
    bgfx_transient_vertex_buffer_t tvb;
    bgfx_transient_index_buffer_t tib;

    bgfx_alloc_transient_vertex_buffer(&tvb, 4, &this->vertex_layout);
    bgfx_alloc_transient_index_buffer(&tib, 6, false);

    neopad_renderer_vertex_t vertices[] = {
            {l, b, 0.0f, 0xff0000ff},
            {l, t, 0.0f, 0xff00ff00},
            {r, t, 0.0f, 0xffff0000},
            {r, b, 0.0f, 0xffffffff},
    };
    memcpy(tvb.data, vertices, sizeof(vertices));

    uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3,
    };
    memcpy(tib.data, indices, sizeof(indices));

    bgfx_set_transient_vertex_buffer(0, &tvb, 0, 4);
    bgfx_set_transient_index_buffer(&tib, 0, 6);

    bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A, 0);
    bgfx_submit(VIEW_CONTENT, this->programs[PROGRAM_BASIC], 0, false);
}


#pragma mark - Line Drawing

void neopad_renderer_pen_begin(neopad_renderer_t this, float x, float y) {

}

void neopad_renderer_pen_draw_to(neopad_renderer_t this, float x, float y) {

}

void neopad_renderer_pen_end(neopad_renderer_t this, float x, float y) {

}