FetchContent_Declare (
        bgfx
        GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
        GIT_TAG "v1.122.8589-457"
        ${NEOPAD_COMMON_DEPS_OPTIONS}
)
set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "")
set(BGFX_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(BGFX_CUSTOM_TARGETS OFF CACHE INTERNAL "")
set(BGFX_CONFIG_MULTITHREADED ON CACHE INTERNAL "")  # todo: rewrite to disable this
FetchContent_MakeAvailable(bgfx)