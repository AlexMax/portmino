#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"
#include "lauxlib.h"

#include "platform.h"
#include "script.h"
#include "vfs.h"

/**
 * A lua cfunction that returns its first upvalue.
 */
static int first_upvalue(lua_State* L) {
    lua_pushvalue(L, lua_upvalueindex(1));
    return 1;
}

static void test_wrap_cfuncs(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    // Create a table with a cfunction and try to wrap it
    lua_pushstring(L, "_ENV");
    lua_createtable(L, 0, 0);
    lua_pushcfunction(L, first_upvalue);
    lua_setfield(L, -2, "first_upvalue");
    int top = lua_gettop(L);
    script_wrap_cfuncs(L, -2);
    assert_true(top == lua_gettop(L));
    lua_setglobal(L, "mino_test");
    lua_pop(L, 1);

    // Call it and ensure it can access the upvalue
    luaL_dostring(L, "return mino_test.first_upvalue()");
    assert_string_equal("_ENV", lua_tostring(L, -1));

    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_wrap_cfuncs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
