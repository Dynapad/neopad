//
// Created by Dylan Lukes on 5/1/23.
//

#ifndef NEOPAD_RENDERER_MODULE_INTERNAL_H
#define NEOPAD_RENDERER_MODULE_INTERNAL_H

#include "bgfx/c99/bgfx.h"

#define NEOPAD_RENDERER_MODULE_BACKGROUND 0
#define NEOPAD_RENDERER_MODULE_COUNT 1

// Forward declaration of the renderer opaque pointer type to avoid circular dependencies.
typedef struct neopad_renderer_s *neopad_renderer_t;

// Forward declaration of module types to avoid circular dependencies.
typedef struct neopad_renderer_module_base_s *neopad_renderer_module_base_t;
typedef struct neopad_renderer_module_background_s *neopad_renderer_module_background_t;
typedef struct neopad_renderer_module_vector_s *neopad_renderer_module_vector_t;

typedef union __attribute__((transparent_union)) neopad_renderer_module_s {
    neopad_renderer_module_base_t base;
    neopad_renderer_module_background_t background;
    neopad_renderer_module_vector_t vector;
} neopad_renderer_module_t;

typedef struct neopad_renderer_module_base_s {
    /// Name of this module.
    const char *name;

    /// View ID (pass/layer) for this module.
    bgfx_view_id_t view_id;

    /// Module hooks
    void (*on_setup)(neopad_renderer_module_t module, neopad_renderer_t renderer);
    void (*on_teardown)(neopad_renderer_module_t module, neopad_renderer_t renderer);
    void (*on_begin_frame)(neopad_renderer_module_t module, neopad_renderer_t renderer);
    void (*on_end_frame)(neopad_renderer_module_t module, neopad_renderer_t renderer);

    /// Render
    void (*render)(neopad_renderer_module_t module, neopad_renderer_t renderer);
} *neopad_renderer_module_base_t;

#endif //NEOPAD_RENDERER_MODULE_INTERNAL_H
