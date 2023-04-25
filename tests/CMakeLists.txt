FetchContent_Declare(
        cmocka
        GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
        GIT_TAG stable-1.1
        GIT_SHALLOW 1)

set(WITH_STATIC_LIB ON CACHE BOOL "CMocka: Build with a static library" FORCE)
set(WITH_CMOCKERY_SUPPORT OFF CACHE BOOL "CMocka: Install a cmockery header" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "CMocka: Build examples" FORCE)
set(UNIT_TESTING OFF CACHE BOOL "CMocka: Build with unit testing" FORCE)
set(PICKY_DEVELOPER OFF CACHE BOOL "CMocka: Build with picky developer flags" FORCE)

FetchContent_MakeAvailable(cmocka)

# Tests need to be added as executables first.
add_executable(neopad_tests test.c)

# Enable C++17 in the tests.
target_compile_features(neopad_tests PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library.
target_link_libraries(neopad_tests PRIVATE neopad cmocka)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME neopad COMMAND neopad_tests) # Command can be a target