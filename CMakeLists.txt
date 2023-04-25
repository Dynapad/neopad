cmake_minimum_required(VERSION 3.14..3.25)

project(neopad
        VERSION 0.1
        DESCRIPTION "Modern C reimplementation of Pad++"
        LANGUAGES C CXX)  # CXX is just for testing...

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

# Add compiled library code.
add_subdirectory(src)

# Add executable demo.
add_subdirectory(demo)

# Add tests (only if this is the main project!)
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()