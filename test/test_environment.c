#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"

#include "environment.h"
#include "platform.h"
#include "script.h"
#include "vfs.h"

/**
 * This test just tests the basics - can we initialize an env and run it
 * through its paces.
 */
static void test_environment(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    environment_t* env = environment_new(L, "stdmino", "endurance");
    assert_non_null(env);

    bool ok = environment_start(env);
    assert_true(ok == true);

    playerinputs_t inputs = { 0 };
    ok = environment_frame(env, &inputs);
    assert_true(ok == true);

    fprintf(stderr, "size: %zu\ndata: ", env->states[0].serialized->size);
    for (size_t i = 0;i < env->states[0].serialized->size;i++) {
        fprintf(stderr, "%02X ", env->states[0].serialized->data[i]);
    }
    fprintf(stderr, "\n");

    ok = environment_rewind(env, 0);
    assert_true(ok == true);

    environment_delete(env);
    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();

    assert_true(error_count() == 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_environment),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
