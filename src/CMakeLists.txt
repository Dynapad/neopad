# Note that headers are optional, and do not affect add_library, but they will not
# show up in some IDEs unless they are listed in add_library.
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${neopad_SOURCE_DIR}/include" *.hpp)

# Can be compiled as static or dynamic based on user setting.
add_library(neopad lib.c ${HEADER_LIST})

# We need this directory, and users of our library will need it too.
target_include_directories(neopad PUBLIC ../include)

# All users of this library will need at least C++11.
target_compile_features(neopad PUBLIC c_std_11)

# IDEs should put the headers in a nice place.
source_group(
        TREE "${PROJECT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${HEADER_LIST})