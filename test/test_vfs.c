#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "physfs.h"

/**
 * Test to make sure PhysFS doesn't leak
 * 
 * This isn't strictly VFS stuff, but whatever.
 */
static void test_physfs(void** state) {
    assert_true(PHYSFS_init(NULL) == true);

    buffer_t* basemino = test_basemino();

    PHYSFS_mountMemory(basemino->data, basemino->size, NULL, "basemino.pk3", NULL, 1);

    assert_true(PHYSFS_deinit() > 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_physfs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
