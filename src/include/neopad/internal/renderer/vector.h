//
// Created by Dylan Lukes on 5/1/23.
//

#ifndef NEOPAD_RENDERER_VECTOR_INTERNAL_H
#define NEOPAD_RENDERER_VECTOR_INTERNAL_H

#include "module.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct neopad_renderer_module_vector_s {
    struct neopad_renderer_module_base_s base;
} *neopad_renderer_module_vector_t;

neopad_renderer_module_t neopad_renderer_module_vector_create(void);

#endif //NEOPAD_RENDERER_VECTOR_INTERNAL_H
