#include <catch2/catch_test_macros.hpp>
#include <neopad/lib.h>

TEST_CASE( "Compiler is happy today", "[main]" ) {
    REQUIRE( 1 + 1 == 2 );
}