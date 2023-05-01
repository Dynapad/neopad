//
// Created by Dylan Lukes on 4/30/23.
//

#include "neopad/renderer.h"
#include "neopad/internal/renderer/vector.h"

#include <memory.h>

void neopad_renderer_begin_points(neopad_renderer_t this, vec2 p) {

}

void neopad_renderer_pen_add_point(neopad_renderer_t this, vec2 p) {

}

void neopad_renderer_end_points(neopad_renderer_t this) {

}

void neopad_renderer_module_vector_destroy(neopad_renderer_module_vector_t module) {
    free(module);
}

neopad_renderer_module_t neopad_renderer_module_vector_create(void) {
    neopad_renderer_module_vector_t module = malloc(sizeof(struct neopad_renderer_module_vector_s));
    memcpy(module, &(struct neopad_renderer_module_vector_s) {
            .base = {
                    .name = "vector",
                    .on_setup = NULL,
                    .on_teardown = NULL,
                    .on_begin_frame = NULL,
                    .on_end_frame = NULL,
                    .render = NULL,
                    .destroy = neopad_renderer_module_vector_destroy
            }
    }, sizeof(struct neopad_renderer_module_vector_s));

    return (neopad_renderer_module_t) { .vector = module };
}
