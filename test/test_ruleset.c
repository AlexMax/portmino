#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "platform.h"
#include "script.h"
#include "ruleset.h"
#include "vfs.h"

static void test_ruleset_new(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    vfs_init(NULL);
    lua_State* L = script_newstate();

    ruleset_t* ruleset = ruleset_new(L, "stdmino");
    assert_non_null(ruleset);
    ruleset_delete(ruleset);

    lua_close(L);
    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ruleset_new),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
