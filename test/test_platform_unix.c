#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

static void test_init(void** state) {
    assert_true(platform_init() == true);
    platform_deinit();
}

static void test_data_dirs(void** state) {
    (void)state;

    putenv("XDG_DATA_HOME=/xdg_data_home");
    putenv("XDG_DATA_DIRS=/xdg_data_dir1:/xdg_data_dir2:/xdg_data_dir3");

    platform_init();
    const char** data_dirs = platform()->data_dirs();
    assert_string_equal("/xdg_data_home/portmino", *data_dirs);
    data_dirs++;
    assert_string_equal("/xdg_data_dir1/portmino", *data_dirs);
    data_dirs++;
    assert_string_equal("/xdg_data_dir2/portmino", *data_dirs);
    data_dirs++;
    assert_string_equal("/xdg_data_dir3/portmino", *data_dirs);
    data_dirs++;
    assert_null(*data_dirs);

    platform_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init),
        cmocka_unit_test(test_data_dirs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
