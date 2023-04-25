#include <catch2/catch_test_macros.hpp>

extern "C" {
#include <neopad/lib.h>
}

TEST_CASE( "Dummy", "[main]" ) {
    REQUIRE( 13 == pad_dummy() );
}