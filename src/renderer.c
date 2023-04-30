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
    float _unused1;
    float _unused2;

    float grid_major;
    float grid_minor;
    float _unused3;
    float _unused4;
} neopad_renderer_uniforms_t;

struct neopad_renderer_s {
    /// BGFX initialization parameters.
    bgfx_init_t init;

    /// Back-buffer resolution.
    uint32_t width;
    uint32_t height;

    /// Intended back-buffer resolution post-resize.
    uint32_t target_width;
    uint32_t target_height;

    /// Content scale. This is used to scale the UI on high-DPI displays.
    float content_scale;

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
    this->width = init.width;
    this->height = init.height;
    this->target_width = init.width;
    this->target_height = init.height;
    this->content_scale = init.content_scale ? init.content_scale : 1.0f;
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

    bgfx_set_debug(init.debug ? BGFX_DEBUG_STATS : 0);

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
            "vs_basic",
            "fs_grid",
            NULL);

    // Initialize uniforms
    this->uniforms = (neopad_renderer_uniforms_t) {
            .time = 0.0f,
            .content_scale = this->content_scale,
            .grid_major = this->background.grid_major,
            .grid_minor = this->background.grid_minor,
    };
    this->uniform_handle = bgfx_create_uniform("u_params", BGFX_UNIFORM_TYPE_VEC4, 1);
    bgfx_set_uniform(this->uniform_handle, &this->uniforms, 1);

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

void neopad_renderer_resize(neopad_renderer_t this, int width, int height) {
    this->target_width = width;
    this->target_height = height;
}

void neopad_renderer_rescale(neopad_renderer_t this, float scale) {
    this->content_scale = scale;
}

void neopad_renderer_destroy(neopad_renderer_t this) {
    bgfx_shutdown();
    free(this);
}

#pragma mark - Rendering

void neopad_renderer_begin_frame(neopad_renderer_t this) {
    if (this->width != this->target_width || this->height != this->target_height) {
        this->width = this->target_width;
        this->height = this->target_height;
        bgfx_reset(this->width, this->height, BGFX_RESET_VSYNC, this->init.resolution.format);
    }

}

void neopad_renderer_end_frame(neopad_renderer_t this) {
    // Set view transform.
    mat4 view;
    mat4 proj;

    float scaled_width = (float) this->width / this->content_scale;
    float scaled_height = (float) this->height / this->content_scale;

    glm_translate_make(view, (vec3) {scaled_width / 2.0f, scaled_height / 2.0f, 0.0f});
    glm_ortho(0.0f, scaled_width, 0.0f, scaled_height, -1.0f, 1.0f, proj);

    // BACKGROUND
    // ----------

    bgfx_set_view_clear(VIEW_BACKGROUND, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, this->background.color, 1.0f, 0);
    bgfx_set_view_transform(VIEW_BACKGROUND, NULL, proj);
    bgfx_set_view_rect(VIEW_BACKGROUND, 0, 0, this->width, this->height);
    bgfx_touch(VIEW_BACKGROUND);

    // CONTENT
    // -------
    bgfx_set_view_transform(VIEW_CONTENT, view, proj);
    bgfx_set_view_rect(VIEW_CONTENT, 0, 0, this->width, this->height);
    bgfx_touch(VIEW_CONTENT);

    bgfx_frame(false);
}

#pragma mark - Drawing

void neopad_renderer_draw_background(neopad_renderer_t this) {
    bgfx_transient_vertex_buffer_t tvb;
    bgfx_transient_index_buffer_t tib;

    bgfx_alloc_transient_vertex_buffer(&tvb, 4, &this->vertex_layout);
    bgfx_alloc_transient_index_buffer(&tib, 6, false);

    // Screen space full quad
    float w = (float) this->width;
    float h = (float) this->height;

    neopad_renderer_vertex_t vertices[] = {
            {0.0f, 0.0f, 0.0f, 0x00000000},
            {w,    0.0f, 0.0f, 0x00000000},
            {w,    h,    0.0f, 0x00000000},
            {0.0f, h,    0.0f, 0x00000000},
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

const float TEST_L = -100.0f;
const float TEST_R = 100.0f;
const float TEST_T = 100.0f;
const float TEST_B = -100.0f;

const neopad_renderer_vertex_t TEST_VERTICES[] = {
        {TEST_L, TEST_B, 0.0f, 0xff0000ff},
        {TEST_L, TEST_T, 0.0f, 0xff00ff00},
        {TEST_R, TEST_T, 0.0f, 0xffff0000},
        {TEST_R, TEST_B, 0.0f, 0xffffffff},
};

const uint16_t TEST_INDICES[] = {
        0, 1, 2,
        0, 2, 3,
};

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