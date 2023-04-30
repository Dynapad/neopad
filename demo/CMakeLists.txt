include(${PROJECT_SOURCE_DIR}/cmake/glfw.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/cglm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/bgfx.cmake)

# Find source files automatically
file(GLOB_RECURSE NEOPAD_DEMO_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR} *.c)

add_executable(neopad_demo ${NEOPAD_DEMO_SOURCES})
set_property(TARGET neopad_demo PROPERTY OUTPUT_NAME neopad)

target_link_libraries(neopad_demo PRIVATE neopad bx bgfx)
target_link_libraries(neopad_demo PRIVATE neopad glfw)
target_link_libraries(neopad_demo PRIVATE neopad cglm)
target_link_libraries(neopad_demo PRIVATE ${SHADERS_TARGET_NAME})