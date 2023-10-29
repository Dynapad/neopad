/*
 * Based on:
 *   - https://github.com/bkaradzic/bgfx/blob/master/include/bgfx/embedded_shader.h
 *   - Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 *   - License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef NEOPAD_SHIMS_BX_EMBEDDED_SHADER_H
#define NEOPAD_SHIMS_BX_EMBEDDED_SHADER_H

#include "bx/platform.h"
#include "bgfx/c99/bgfx.h"

// Fills for macros missing from C99 BGFX.
#ifndef BX_CONCATENATE
#define BX_CONCATENATE(_x, _y) BX_CONCATENATE_(_x, _y)
#define BX_CONCATENATE_(_x, _y) _x ## _y
#endif // BX_CONCATENATE

#ifndef BX_COUNTOF
#define BX_COUNTOF(x) ((sizeof(x) / sizeof(*(x))) - 1)
#endif

#define BGFX_EMBEDDED_SHADER_DXBC(...)
#define BGFX_EMBEDDED_SHADER_DX9BC(...)
#define BGFX_EMBEDDED_SHADER_PSSL(...)
#define BGFX_EMBEDDED_SHADER_ESSL(...)
#define BGFX_EMBEDDED_SHADER_GLSL(...)
#define BGFX_EMBEDDED_SHADER_METAL(...)
#define BGFX_EMBEDDED_SHADER_NVN(...)
#define BGFX_EMBEDDED_SHADER_SPIRV(...)

#define BGFX_PLATFORM_SUPPORTS_DX9BC (0 \
    || BX_PLATFORM_WINDOWS              \
    )
#define BGFX_PLATFORM_SUPPORTS_DXBC (0  \
    || BX_PLATFORM_WINRT                \
    || BX_PLATFORM_XBOXONE              \
    )
#define BGFX_PLATFORM_SUPPORTS_PSSL (0  \
    || BX_PLATFORM_PS4                  \
    || BX_PLATFORM_PS5                  \
    )
#define BGFX_PLATFORM_SUPPORTS_ESSL (0  \
    || BX_PLATFORM_ANDROID              \
    || BX_PLATFORM_EMSCRIPTEN           \
    || BX_PLATFORM_IOS                  \
    || BX_PLATFORM_LINUX                \
    || BX_PLATFORM_OSX                  \
    || BX_PLATFORM_RPI                  \
    || BX_PLATFORM_WINDOWS              \
    )
#define BGFX_PLATFORM_SUPPORTS_GLSL (0  \
    || BX_PLATFORM_BSD                  \
    || BX_PLATFORM_LINUX                \
    || BX_PLATFORM_OSX                  \
    || BX_PLATFORM_WINDOWS              \
    )
#define BGFX_PLATFORM_SUPPORTS_METAL (0 \
    || BX_PLATFORM_IOS                  \
    || BX_PLATFORM_OSX                  \
    )
#define BGFX_PLATFORM_SUPPORTS_NVN (0   \
    || BX_PLATFORM_NX                   \
    )
#define BGFX_PLATFORM_SUPPORTS_SPIRV (0 \
    || BX_PLATFORM_ANDROID              \
    || BX_PLATFORM_EMSCRIPTEN           \
    || BX_PLATFORM_LINUX                \
    || BX_PLATFORM_WINDOWS              \
    || BX_PLATFORM_OSX                  \
    )

#if BGFX_PLATFORM_SUPPORTS_DX9BC
#	undef  BGFX_EMBEDDED_SHADER_DX9BC
#	define BGFX_EMBEDDED_SHADER_DX9BC(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _dx9 ), BX_COUNTOF(BX_CONCATENATE(_name, _dx9 ) ) },
#endif // BGFX_PLATFORM_SUPPORTS_DX9BC

#if BGFX_PLATFORM_SUPPORTS_DXBC
#	undef  BGFX_EMBEDDED_SHADER_DXBC
#	define BGFX_EMBEDDED_SHADER_DXBC(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _dx11), BX_COUNTOF(BX_CONCATENATE(_name, _dx11) ) },
#endif // BGFX_PLATFORM_SUPPORTS_DXBC

#if BGFX_PLATFORM_SUPPORTS_PSSL
#	undef  BGFX_EMBEDDED_SHADER_PSSL
#	define BGFX_EMBEDDED_SHADER_PSSL(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _pssl), BX_CONCATENATE(_name, _pssl_size) },
#endif // BGFX_PLATFORM_SUPPORTS_PSSL

#if BGFX_PLATFORM_SUPPORTS_ESSL
#	undef  BGFX_EMBEDDED_SHADER_ESSL
#	define BGFX_EMBEDDED_SHADER_ESSL(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _essl), BX_COUNTOF(BX_CONCATENATE(_name, _essl) ) },
#endif // BGFX_PLATFORM_SUPPORTS_ESSL

#if BGFX_PLATFORM_SUPPORTS_GLSL
#	undef  BGFX_EMBEDDED_SHADER_GLSL
#	define BGFX_EMBEDDED_SHADER_GLSL(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _glsl), BX_COUNTOF(BX_CONCATENATE(_name, _glsl) ) },
#endif // BGFX_PLATFORM_SUPPORTS_GLSL

#if BGFX_PLATFORM_SUPPORTS_SPIRV
#	undef  BGFX_EMBEDDED_SHADER_SPIRV
#	define BGFX_EMBEDDED_SHADER_SPIRV(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _spv), BX_COUNTOF(BX_CONCATENATE(_name, _spv) ) },
#endif // BGFX_PLATFORM_SUPPORTS_SPIRV

#if BGFX_PLATFORM_SUPPORTS_METAL
#	undef  BGFX_EMBEDDED_SHADER_METAL
#	define BGFX_EMBEDDED_SHADER_METAL(_renderer, _name) \
    { _renderer, BX_CONCATENATE(_name, _mtl), BX_COUNTOF(BX_CONCATENATE(_name, _mtl) ) },
#endif // BGFX_PLATFORM_SUPPORTS_METAL

#define BGFX_EMBEDDED_SHADER(_name)                                                        \
    {                                                                                      \
        #_name,                                                                            \
        {                                                                                  \
            BGFX_EMBEDDED_SHADER_PSSL (BGFX_RENDERER_TYPE_AGC,        _name)              \
            BGFX_EMBEDDED_SHADER_DX9BC(BGFX_RENDERER_TYPE_DIRECT3D9,  _name)              \
            BGFX_EMBEDDED_SHADER_DXBC (BGFX_RENDERER_TYPE_DIRECT3D11, _name)              \
            BGFX_EMBEDDED_SHADER_DXBC (BGFX_RENDERER_TYPE_DIRECT3D12, _name)              \
            BGFX_EMBEDDED_SHADER_PSSL (BGFX_RENDERER_TYPE_GNM,        _name)              \
            BGFX_EMBEDDED_SHADER_METAL(BGFX_RENDERER_TYPE_METAL,      _name)              \
            BGFX_EMBEDDED_SHADER_NVN  (BGFX_RENDERER_TYPE_NVN,        _name)              \
            BGFX_EMBEDDED_SHADER_ESSL (BGFX_RENDERER_TYPE_OPENGLES,   _name)              \
            BGFX_EMBEDDED_SHADER_GLSL (BGFX_RENDERER_TYPE_OPENGL,     _name)              \
            BGFX_EMBEDDED_SHADER_SPIRV(BGFX_RENDERER_TYPE_VULKAN,     _name)              \
            BGFX_EMBEDDED_SHADER_SPIRV(BGFX_RENDERER_TYPE_WEBGPU,     _name)              \
            { BGFX_RENDERER_TYPE_NOOP,  (const uint8_t*)"VSH\x5\x0\x0\x0\x0\x0\x0", 10 }, \
            { BGFX_RENDERER_TYPE_COUNT, NULL, 0 }                                         \
        }                                                                                  \
    }

#define BGFX_EMBEDDED_SHADER_END()                 \
    {                                              \
        NULL,                                      \
        {                                          \
            { BGFX_RENDERER_TYPE_COUNT, NULL, 0 } \
        }                                          \
    }

typedef struct bgfx_embedded_shader_data_s {
    enum bgfx_renderer_type type;
    const uint8_t *data;
    uint32_t size;
} bgfx_embedded_shader_data_t;

typedef struct bgfx_embedded_shader_s {
    const char *name;
    bgfx_embedded_shader_data_t data[BGFX_RENDERER_TYPE_COUNT];
} bgfx_embedded_shader_t;

/// Create shader from embedded shader data.
///
/// @param[in] _es Pointer to `BGFX_EMBEDDED_SHADER` data.
/// @param[in] _type Renderer backend type. See: `bgfx_renderer_type`
/// @param[in] _name Shader name.
/// @returns Shader handle.
///
bgfx_shader_handle_t bgfx_create_embedded_shader(
        const bgfx_embedded_shader_t *_es,
        enum bgfx_renderer_type _type,
        const char *_name
);

/// Create program from embedded shader data.
bgfx_program_handle_t bgfx_create_embedded_program(
        const bgfx_embedded_shader_t *_es,
        enum bgfx_renderer_type _type,
        const char *_vsName,
        const char *_fsName,
        bool _destroyShaders
);

#endif // NEOPAD_SHIMS_BX_EMBEDDED_SHADER_H
