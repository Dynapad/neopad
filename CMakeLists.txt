cmake_minimum_required(VERSION 3.14..3.25)

# Fix behavior of CMAKE_C_STANDARD when targeting macOS.
if (POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif ()

project(neopad
        VERSION 0.1
        DESCRIPTION "Modern C reimplementation of Pad++"
        LANGUAGES C CXX)

# Plugin for adding shaders to the build.
include(cmake/bgfx-shaders.cmake)

# Only do these if this is the main project, and not if it is included through add_subdirectory.
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Require C17 (C11 + defect fixes)
    set(CMAKE_C_STANDARD 17)

    # Ensure (GNU) extensions are disabled.
    set(CMAKE_C_EXTENSIONS OFF)

    # Nicely support folders in IDEs.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Enable testing.
    include(CTest)

    find_package(Doxygen)
    if (Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(STATUS "Doxygen not found, skipping building docs.")
    endif ()
endif ()

# FetchContent added in CMake 3.11, downloads during the configure step
# FetchContent_MakeAvailable was added in CMake 3.14; simpler usage
include(FetchContent)
set(FetchContent_QUIET FALSE)

# Options we want to use for all FetchContent calls.
set(NEOPAD_COMMON_DEPS_OPTIONS)
list(APPEND NEOPAD_COMMON_DEPS_OPTIONS GIT_SHALLOW TRUE)
list(APPEND NEOPAD_COMMON_DEPS_OPTIONS GIT_PROGRESS TRUE)
list(APPEND NEOPAD_COMMON_DEPS_OPTIONS USES_TERMINAL_DOWNLOAD TRUE)
list(APPEND NEOPAD_COMMON_DEPS_OPTIONS GIT_REMOTE_UPDATE_STRATEGY CHECKOUT)
list(APPEND NEOPAD_COMMON_DEPS_OPTIONS GIT_SUBMODULES_RECURSE TRUE)

# Add compiled library code.
add_subdirectory(src)

# Add executable demo.
add_subdirectory(demo)

# Add tests (only if this is the main project!)
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif()