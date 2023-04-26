FetchContent_Declare (
        bgfx
        GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
        GIT_TAG "v1.118.8417-420"
        PATCH_COMMAND git reset --hard && git apply --ignore-space-change --ignore-whitespace --allow-empty "${PROJECT_SOURCE_DIR}/cmake/patches/bgfx-cmake.patch"
        ${NEOPAD_COMMON_DEPS_OPTIONS}
)
set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "")
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_CUSTOM_TARGETS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(bgfx)