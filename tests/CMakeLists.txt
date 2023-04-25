FetchContent_Declare(
        catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.3.2)
FetchContent_MakeAvailable(catch)

# Tests need to be added as executables first.
add_executable(neopad_tests test.cpp)

# Enable C++17 in the tests.
target_compile_features(neopad_tests PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library.
target_link_libraries(neopad_tests PRIVATE neopad Catch2::Catch2WithMain)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME neopad COMMAND neopad_tests) # Command can be a target