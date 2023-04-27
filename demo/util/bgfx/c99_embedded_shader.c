//
// Created by Dylan Lukes on 4/26/23.
//

#include "c99_embedded_shader.h"

#include <string.h>

bgfx_shader_handle_t bgfx_create_embedded_shader(
        const bgfx_embedded_shader_t *_es,
        enum bgfx_renderer_type _type,
        const char *_name
) {
    int _name_len = strlen(_name);
    const bgfx_embedded_shader_t *es;

    for (es = _es; NULL != es->name; ++es) {
        if (0 == strncmp(_name, es->name, _name_len)) {
            const bgfx_embedded_shader_data_t *esd = es->data;
            for (esd = es->data; BGFX_RENDERER_TYPE_COUNT != esd->type; ++esd) {
                if (_type == esd->type && 1 < esd->size) {
                    bgfx_shader_handle_t shader_handle;
                    shader_handle = bgfx_create_shader(bgfx_make_ref(esd->data, esd->size));
                    if (BGFX_HANDLE_IS_VALID(shader_handle)) {
                        bgfx_set_shader_name(shader_handle, _name, _name_len);
                    }
                    return shader_handle;
                }
            }
        }
    }

    bgfx_shader_handle_t invalid_handle = BGFX_INVALID_HANDLE;
    return invalid_handle;
}