include(${PROJECT_SOURCE_DIR}/cmake/glfw.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cglm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/bgfx.cmake)

# OpenGL
find_package(OpenGL REQUIRED)

# Find source files automatically
file(GLOB_RECURSE NEOPAD_DEMO_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR} *.c)

# Shaders
add_shaders_directory(shaders SHADERS_TARGET_NAME)

add_executable(neopad_demo ${NEOPAD_DEMO_SOURCES})
set_property(TARGET neopad_demo PROPERTY OUTPUT_NAME neopad)

target_link_libraries(neopad_demo PRIVATE neopad
        OpenGL::GL
        bx
        bgfx
        glfw
        cglm)
target_link_libraries(neopad_demo PUBLIC ${SHADERS_TARGET_NAME})