#include <neopad/lib.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

static void test_dummy(void **state) {
    assert_int_equal(13, pad_dummy());
}

int main() {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_dummy)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}