//
// Created by Dylan Lukes on 5/1/23.
//
//

#ifndef NEOPAD_RENDERER_BACKGROUND_INTERNAL_H
#define NEOPAD_RENDERER_BACKGROUND_INTERNAL_H

#include "module.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct neopad_renderer_module_background_s {
    struct neopad_renderer_module_base_s base;

    uint32_t color;
    bool grid_enabled;
    float grid_major;
    float grid_minor;

    // todo: use these instead of a transient (avoid copies)
    bgfx_vertex_buffer_handle_t vbo;
    bgfx_index_buffer_handle_t ibo;
} *neopad_renderer_module_background_t;

neopad_renderer_module_t neopad_renderer_module_background_create(bgfx_view_id_t view_id, uint32_t color, bool grid_enabled, float grid_major, float grid_minor);

#endif //NEOPAD_RENDERER_BACKGROUND_INTERNAL_H
