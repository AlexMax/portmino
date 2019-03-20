#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"

#include "error.h"
#include "platform.h"
#include "script.h"
#include "scriptenv.h"
#include "vfs.h"

/**
 * This test just tests the basics - can we initialize an env and run it
 * through its paces.
 */
static void test_scriptenv(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    scriptenv_t* env = scriptenv_new(L, "stdmino", "endurance");
    assert_non_null(env);

    bool ok = scriptenv_start(env);
    assert_true(ok == true);

    scriptenv_delete(env);
    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_scriptenv),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
