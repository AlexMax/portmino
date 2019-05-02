#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "platform.h"
#include "softblock.h"
#include "vfs.h"

static void test_softblock(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    softblock_t* block = softblock_new("block/default/8px.png");
    assert_true(block != NULL);

    softblock_delete(block);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_softblock),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
