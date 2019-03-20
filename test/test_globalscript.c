#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"
#include "lauxlib.h"

#include "environment.h"
#include "platform.h"
#include "script.h"
#include "vfs.h"

static void test_globalscript_doconfig(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    // Should return an error - missing internal state
    luaL_dostring(L, "return doconfig('pieces');");
    assert_true(lua_isstring(L, -1) == 1);
    lua_pop(L, 1);

    // Should return a table with our pieces
    environment_t* env = environment_new(L, "stdmino", "endurance");
    assert_non_null(env);
    luaL_loadstring(L, "return doconfig('pieces');");
    lua_rawgeti(L, LUA_REGISTRYINDEX, env->env_ref);
    lua_setupvalue(L, -2, 1); // pop environment
    lua_pcall(L, 0, 1, 0);
    assert_true(lua_istable(L, -1) == 1);
    lua_getfield(L, -1, "l_piece");
    assert_true(lua_istable(L, -1) == 1);
    lua_pop(L, 1); // pop l_piece table
    lua_pop(L, 1); // pop pieces table

    environment_delete(env);
    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_globalscript_doconfig),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
