//
// Created by Dylan Lukes on 5/1/23.
//

#include "neopad/internal/renderer.h"
#include "neopad/internal/renderer/background.h"

#include <memory.h>

static const neopad_renderer_vertex_t NDC_QUAD_VERTICES[] = {
        {-1, 1,  0,  0},
        {1,  1,  0, 0},
        {1,  -1, 0,  0},
        {-1, -1, 0,  0},
};

uint16_t NDC_QUAD_INDICES[] = {
        0, 1, 2,
        0, 2, 3,
};

void neopad_renderer_background_module_on_setup(neopad_renderer_module_t module, neopad_renderer_t renderer) {
    neopad_renderer_module_background_t this = module.background;

    renderer->programs[PROGRAM_BACKGROUND] = bgfx_create_embedded_program(
            embedded_shaders,
            bgfx_get_renderer_type(),
            "vs_grid",
            "fs_grid",
            NULL);

    this->vbo = bgfx_create_vertex_buffer(
            bgfx_make_ref(NDC_QUAD_VERTICES, sizeof(NDC_QUAD_VERTICES)),
            &renderer->vertex_layout,
            BGFX_BUFFER_NONE);
    this->ibo = bgfx_create_index_buffer(
            bgfx_make_ref(NDC_QUAD_INDICES, sizeof(NDC_QUAD_INDICES)),
            BGFX_BUFFER_NONE);
}

void neopad_renderer_background_module_on_teardown(neopad_renderer_module_t module, neopad_renderer_t renderer) {
    // No teardown required.
}

void neopad_renderer_background_module_on_begin_frame(neopad_renderer_module_t module, neopad_renderer_t renderer) {
    renderer->uniforms.grid_major = module.background->grid_major;
    renderer->uniforms.grid_minor = module.background->grid_minor;
}

void neopad_renderer_background_module_on_render(neopad_renderer_module_t module, neopad_renderer_t renderer) {
    neopad_renderer_module_background_t this = module.background;
    bgfx_view_id_t view_id = module.base->view_id;

    if (!this->grid_enabled) {
        return;
    }

    bgfx_set_vertex_buffer(0, this->vbo, 0, 4);
    bgfx_set_index_buffer(this->ibo, 0, 6);

    bgfx_set_view_clear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, this->color, 1.0f, 0);

    bgfx_set_state(BGFX_STATE_WRITE_RGB
                   | BGFX_STATE_WRITE_A
                   | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_DST_ALPHA),
                   0);

    bgfx_submit(view_id, renderer->programs[PROGRAM_BACKGROUND], 0, false);
}

void neopad_renderer_background_module_on_end_frame(neopad_renderer_module_t module, neopad_renderer_t renderer) {
    // The background uses the same view and projection matrices as the content, but
    // it does not use them in the same way. The background is always drawn at the same
    // size, regardless of the content scale or zoom. It renders geometry provided in
    // clip-space coordinates. It needs the same view and projection matrices, so it can
    // invert them and use them to transform the clip-space coordinates into world-space
    // coordinates. This allows the background to be drawn in the same world-space as the
    // content, but at a fixed size.

    neopad_renderer_module_background_t this = module.background;
    bgfx_view_id_t view_id = module.base->view_id;

    bgfx_set_view_clear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, this->color, 1.0f, 0);
    bgfx_set_view_transform(view_id, renderer->model_view, renderer->proj);
    bgfx_set_view_rect(view_id, 0, 0, renderer->width, renderer->height);
    bgfx_touch(view_id);
}

void neopad_renderer_module_background_destroy(neopad_renderer_module_background_t module) {
    free(module);
}

neopad_renderer_module_t neopad_renderer_module_background_create(bgfx_view_id_t view_id) {
    neopad_renderer_module_background_t module = malloc(sizeof(struct neopad_renderer_module_background_s));

    memcpy(module, &(struct neopad_renderer_module_background_s) {
            .base = {
                    .name = "background",
                    .view_id = view_id,
                    .on_setup = neopad_renderer_background_module_on_setup,
                    .on_teardown = neopad_renderer_background_module_on_teardown,
                    .on_begin_frame = neopad_renderer_background_module_on_begin_frame,
                    .on_end_frame = neopad_renderer_background_module_on_end_frame,
                    .render = neopad_renderer_background_module_on_render,
                    .destroy = neopad_renderer_module_background_destroy,
            },
            .color = 0x333333ff,
            .grid_enabled = true,
            .grid_major = 100.0f,
            .grid_minor = 25.0f
    }, sizeof(struct neopad_renderer_module_background_s));

    return (neopad_renderer_module_t) {.background = module};
}