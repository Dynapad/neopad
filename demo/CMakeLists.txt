# Graphics Library Framework
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# C Graphics Library Math
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm.git
        GIT_TAG v0.8.9)
FetchContent_MakeAvailable(cglm)

# OpenGL
find_package(OpenGL REQUIRED)

add_executable(neopad_demo
        demo.c
        shaders.c)
set_property(TARGET neopad_demo PROPERTY OUTPUT_NAME neopad)

target_link_libraries(neopad_demo PRIVATE neopad
        OpenGL::GL
        glfw
        cglm)