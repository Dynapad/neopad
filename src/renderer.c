//
// Created by Dylan Lukes on 4/28/23.
//
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <memory.h>

#include "neopad/types.h"
#include "neopad/renderer.h"
#include "neopad/internal/log.h"
#include "neopad/internal/renderer.h"
#include "neopad/internal/renderer/background.h"
#include "neopad/internal/renderer/vector.h"
#include "neopad/internal/shims/bx/thread.h"

#pragma mark - Lifecycle

int api_thread_entry(bx_thread_t self, void *user_data) {
    bx_thread_set_name(self, "bgfx-api-thread");

    return 0;
}


neopad_renderer_t neopad_renderer_create() {
    neopad_renderer_t renderer = malloc(sizeof(struct neopad_renderer_s));
    memset(renderer, 0, sizeof(struct neopad_renderer_s));
    return renderer;
}

void neopad_renderer_init(neopad_renderer_t this, neopad_renderer_init_t init) {
    this->init = init;

    // Populate ourselves
    this->width = this->target_width = this->init.width;
    this->height = this->target_height = this->init.height;
    this->content_scale = this->init.content_scale > 0 ? this->init.content_scale : 1.0f;
    glm_vec2_zero(this->camera);
    glm_vec2_zero(this->target_camera);
    this->zoom = this->target_zoom = 1.0f;

    // Populate modules
    this->modules[NEOPAD_RENDERER_MODULE_BACKGROUND] = neopad_renderer_module_background_create(
            NEOPAD_VIEW_BACKGROUND,
            this->init.background.color,
            this->init.background.grid_enabled,
            this->init.background.grid_major,
            this->init.background.grid_minor);
    this->modules[NEOPAD_RENDERER_MODULE_VECTOR] = neopad_renderer_module_vector_create();

    // Set up the API thread.
    this->render_thread = bx_thread_create();
    bx_thread_init(this->render_thread, api_thread_entry, this, 0, NULL);

    // Initialize BGFX
    bgfx_init_ctor(&this->bgfx_init);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
    if (BX_PLATFORM_WINDOWS) {
        this->bgfx_init.type = BGFX_RENDERER_TYPE_DIRECT3D9;
    }
#pragma clang diagnostic pop
    this->bgfx_init.resolution.width = this->init.width;
    this->bgfx_init.resolution.height = this->init.height;
    this->bgfx_init.platformData.nwh = this->init.native_window_handle;
    this->bgfx_init.platformData.ndt = this->init.native_display_type;

    if (!bgfx_init(&this->bgfx_init)) {
        eprintf("Failed to initialize BGFX.\n");
        exit(EXIT_FAILURE);
    }

    // Initial reset.
    const uint32_t reset_flags = BGFX_RESET_VSYNC;
    bgfx_reset(this->width, this->height, reset_flags, this->bgfx_init.resolution.format);
    bgfx_set_debug(this->init.debug ? BGFX_DEBUG_TEXT : 0);

    // Initialize vertex layout
    bgfx_vertex_layout_begin(&this->vertex_layout, BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&this->vertex_layout, BGFX_ATTRIB_POSITION, 4, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_layout_add(&this->vertex_layout, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true, false);
    bgfx_vertex_layout_end(&this->vertex_layout);

    // Initialize basic program (others handled in modules)
    bgfx_renderer_type_t renderer_type = bgfx_get_renderer_type();
    this->programs[NEOPAD_PROGRAM_BASIC] = bgfx_create_embedded_program(
            embedded_shaders,
            renderer_type,
            "vs_basic",
            "fs_basic",
            NULL);

    // Initialize uniforms (others handled in modules)
    this->uniforms = (neopad_renderer_uniforms_t) {
            .time = 0.0f,
            .zoom = this->zoom
    };
    this->uniform_handle = bgfx_create_uniform("u_params", BGFX_UNIFORM_TYPE_VEC4, 2);

    // Per-module setup
    for (int i = 0; i < NEOPAD_RENDERER_MODULE_COUNT; i++) {
        neopad_renderer_module_t mod = this->modules[i];
        if (mod.base->on_setup) {
            mod.base->on_setup(mod, this);
        }
    }
}

void neopad_renderer_shutdown(neopad_renderer_t this) {
    // Per-module teardown, in reverse setup order.
    for (int i = NEOPAD_RENDERER_MODULE_COUNT - 1; i >= 0; i--) {
        neopad_renderer_module_t mod = this->modules[i];
        if (mod.base->on_teardown) {
            mod.base->on_teardown(mod, this);
        }
    }

    bgfx_destroy_uniform(this->uniform_handle);
    bgfx_destroy_program(this->programs[NEOPAD_PROGRAM_BACKGROUND]);
    bgfx_destroy_program(this->programs[NEOPAD_PROGRAM_BASIC]);
    bgfx_shutdown();
}

void neopad_renderer_destroy(neopad_renderer_t this) {
    // Per-module destruction, in reverse creation order
    for (int i = NEOPAD_RENDERER_MODULE_COUNT - 1; i >= 0; i--) {
        neopad_renderer_module_t mod = this->modules[i];
        if (mod.base->destroy) {
            mod.base->destroy(mod);
        }
    }
    free(this);
}

#pragma mark - Coordinate Transformations

void neopad_renderer_window_to_world(neopad_renderer_const_t this,
                                     const neopad_vec4_t viewport,
                                     const neopad_vec2_t p,
                                     neopad_vec2_t *q) {
    // Create the transforms we need.
    mat4 viewport_to_ndc;
    mat4 inv_model_view;
    mat4 inv_proj;
    glm_ortho(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.0f, 1.0f, viewport_to_ndc);

    glm_mat4_inv(this->model_view, inv_model_view);
    glm_mat4_inv(this->proj, inv_proj);

    vec4 v = {p.x, p.y, 0.0f, 1.0f};
    vec4 w;

    glm_mat4_mulv(viewport_to_ndc, v, w);
    glm_mat4_mulv(inv_proj, w, w);
    glm_mat4_mulv(inv_model_view, w, w);

    glm_vec2_copy((vec2) {w[0], w[1]}, q->vec);
}

void
neopad_renderer_window_to_screen(neopad_renderer_const_t this,
                                 const neopad_vec4_t viewport,
                                 const neopad_vec2_t p,
                                 neopad_vec2_t *q) {
    // Create the transforms we need.
    mat4 viewport_to_ndc;
    mat4 inv_model;
    mat4 inv_proj;
    glm_ortho(viewport.left, viewport.right, viewport.bottom, viewport.top, -1.0f, 1.0f, viewport_to_ndc);

    glm_mat4_inv(this->model, inv_model);
    glm_mat4_inv(this->proj, inv_proj);

    vec4 v = {p.x, p.y, 0.0f, 1.0f};
    vec4 w;

    glm_mat4_mulv(viewport_to_ndc, v, w);
    glm_mat4_mulv(inv_proj, w, w);
    glm_mat4_mulv(inv_model, w, w);

    glm_vec2_copy((vec2) {w[0], w[1]}, q->vec);
}

#pragma mark - Manipualtion

void neopad_renderer_resize(neopad_renderer_t this, int width, int height) {
    this->target_width = width;
    this->target_height = height;
}

void neopad_renderer_rescale(neopad_renderer_t this, float content_scale) {
    this->content_scale = content_scale;
}

void neopad_renderer_zoom(neopad_renderer_t this, float zoom) {
    this->target_zoom = zoom;
}

float neopad_renderer_arrest_zoom(neopad_renderer_t this) {
    this->target_zoom = this->zoom;
    return this->zoom;
}

void neopad_renderer_get_camera(neopad_renderer_const_t this, neopad_vec2_t *dst) {
    glm_vec2_copy(this->camera, dst->vec);
}

void neopad_renderer_set_camera(neopad_renderer_t this, neopad_vec2_t src) {
    glm_vec2_copy(src.vec, this->target_camera);
}

#pragma mark - Frames

void neopad_renderer_await_frame(neopad_renderer_t this, int timeout_ms) {
    bgfx_render_frame(timeout_ms);
}

void neopad_renderer_begin_frame(neopad_renderer_t this) {
    float SMOOTHNESS = 50.0f * this->content_scale;

    // Calculate and update delta time.
    const bgfx_stats_t *stats = bgfx_get_stats();
    const double freq = (double) stats->cpuTimerFreq;
    const float to_ms = 1000.0f / (float) freq;
    const float delta_t = (float) stats->cpuTimeFrame * to_ms;

    if (this->width != this->target_width || this->height != this->target_height) {
        this->width = this->target_width;
        this->height = this->target_height;
        const uint32_t reset_flags = BGFX_RESET_VSYNC;
        bgfx_reset(this->width, this->height, reset_flags, this->bgfx_init.resolution.format);
    }

    if (!glm_vec2_eqv(this->camera, this->target_camera)) {
        vec2 delta_camera;
        vec2 interp_camera;
        glm_vec2_sub(this->target_camera, this->camera, delta_camera);
        glm_vec2_scale(delta_camera, delta_t / SMOOTHNESS, interp_camera);
        glm_vec2_add(this->camera, interp_camera, interp_camera);
        glm_vec2_copy(interp_camera, this->camera);
    }

    float zoom_tol = 0.03f;
    if (fabsf(this->zoom - this->target_zoom) < zoom_tol) {
        this->zoom = this->target_zoom;
        this->uniforms.zoom = this->zoom;
    } else {
        float delta_zoom = this->target_zoom - this->zoom;
        float interp_zoom = delta_zoom * (delta_t / SMOOTHNESS);
        this->zoom += interp_zoom;
        this->uniforms.zoom = this->zoom;
    }

    // Per-module begin frame.
    for (int i = 0; i < NEOPAD_RENDERER_MODULE_COUNT; i++) {
        neopad_renderer_module_t mod = this->modules[i];
        if (mod.base->on_begin_frame) {
            mod.base->on_begin_frame(mod, this);
        }
    }

    // Update any uniforms that have changed (or really, just all of them).
    bgfx_set_uniform(this->uniform_handle, &this->uniforms, 2);
}

void neopad_renderer_end_frame(neopad_renderer_t this) {
    float width = (float) this->width;
    float height = (float) this->height;
    float zoom = this->zoom;

    vec3 camera = {0.0f, 0.0f, 0.0f};
    glm_vec2_copy(this->camera, camera);

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

    bgfx_set_view_transform(NEOPAD_VIEW_CONTENT, this->model_view, this->proj);
    bgfx_set_view_rect(NEOPAD_VIEW_CONTENT, 0, 0, this->width, this->height);
    bgfx_touch(NEOPAD_VIEW_CONTENT);

    for (int i = 0; i < NEOPAD_RENDERER_MODULE_COUNT; i++) {
        neopad_renderer_module_t mod = this->modules[i];
        if (mod.base->on_end_frame) {
            mod.base->on_end_frame(mod, this);
        }
    }

    if (this->init.debug) {
        const bgfx_stats_t *stats = bgfx_get_stats();

        const double freq = (double) stats->cpuTimerFreq;
        const double toMs = 1000.0f / (double) freq;
        const double frameTime = (double) stats->cpuTimeEnd - (double) stats->cpuTimeBegin;

        bgfx_dbg_text_clear(0, false);
        bgfx_dbg_text_printf(0, 0, 0x0f, "   CPU FPS: %.2f", freq / frameTime);
        bgfx_dbg_text_printf(0, 1, 0x0f, "Delta Time: %.2fms", frameTime * toMs);
        bgfx_dbg_text_printf(0, 5, 0x0f, "    Camera: (%f, %f)", camera[0], camera[1]);
        bgfx_dbg_text_printf(0, 6, 0x0f, "      Zoom: %f -> %f", this->zoom, this->target_zoom);
        bgfx_dbg_text_printf(0, 7, 0x0f, "     Scale: %f", this->content_scale);
    }

    bgfx_frame(false);
}

#pragma mark - Drawing

void neopad_renderer_draw_background(neopad_renderer_t this) {
    neopad_renderer_module_t mod = this->modules[NEOPAD_RENDERER_MODULE_BACKGROUND];
    mod.base->render(mod, this);
}

#pragma mark - Testing

void neopad_renderer_draw_test_rect(neopad_renderer_t this, float l, float t, float r, float b) {
    bgfx_transient_vertex_buffer_t tvb;
    bgfx_transient_index_buffer_t tib;

    bgfx_alloc_transient_vertex_buffer(&tvb, 4, &this->vertex_layout);
    bgfx_alloc_transient_index_buffer(&tib, 6, false);

    // Linear map the value of this->zoom from [1, 10] to [0.8, 0.0].
    // For values less than 1.0, the alpha will be 1.0.
    uint8_t a = this->zoom < 1 ? 0xCC : (uint8_t) (255.0f * (0.8f - (this->zoom - 1.0f) / 9.0f));
    uint32_t alpha = a << 24;

    neopad_renderer_vertex_t vertices[] = {
            {l, t, 0, 1, alpha | 0x0000ff00},
            {r, t, 0, 1, alpha | 0x00ff0000},
            {r, b, 0, 1, alpha | 0x00ffffff},
            {l, b, 0, 1, alpha | 0x000000ff},
    };
    memcpy(tvb.data, vertices, sizeof(vertices));

    uint16_t indices[] = {
            0, 1, 2,
            0, 2, 3,
    };
    memcpy(tib.data, indices, sizeof(indices));

    bgfx_set_transient_vertex_buffer(0, &tvb, 0, 4);
    bgfx_set_transient_index_buffer(&tib, 0, 6);

    bgfx_set_state(BGFX_STATE_WRITE_RGB
                   | BGFX_STATE_WRITE_A
                   | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA), 0);
    bgfx_submit(NEOPAD_VIEW_CONTENT, this->programs[NEOPAD_PROGRAM_BASIC], 0, false);
}
