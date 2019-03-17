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

static void test_scriptenv_new(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    scriptenv_t* env = scriptenv_new(L, "stdmino", "endurance");
    error_debug();
    assert_non_null(env);

    scriptenv_delete(env);
    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_scriptenv_new),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
