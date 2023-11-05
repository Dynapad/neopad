include(${PROJECT_SOURCE_DIR}/cmake/cglm.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/bgfx.cmake)

# Find source files automatically
file(GLOB_RECURSE NEOPAD_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Note that headers are optional, and do not affect add_library, but they will not
# show up in some IDEs unless they are listed in add_library.
file(GLOB_RECURSE NEOPAD_HEADERS CONFIGURE_DEPENDS
        "${neopad_SOURCE_DIR}/include/*.h")

# Can be compiled as static or dynamic based on user setting.
add_library(neopad ${NEOPAD_SOURCES} ${NEOPAD_HEADERS})

# Compile shaders into headers for embedding, output to "build/src/include/generated/...".
add_shaders_directory("renderer/shaders" SHADERS_TARGET_NAME)

# We need this directory, and users of our library will need it too.
target_include_directories(neopad PUBLIC ../include)
target_include_directories(neopad PRIVATE include)

# All users of this library will need at least C11.
target_compile_features(neopad PUBLIC c_std_11)

# IDEs should put the headers in a nice place.
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${NEOPAD_HEADERS})

target_link_libraries(neopad PRIVATE bx bgfx)
target_link_libraries(neopad PRIVATE cglm)
target_link_libraries(neopad PRIVATE ${SHADERS_TARGET_NAME})