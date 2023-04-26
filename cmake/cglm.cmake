FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG v0.8.9
        ${NEOPAD_COMMON_DEPS_OPTIONS}
)
FetchContent_MakeAvailable(cglm)