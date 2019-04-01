#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"
#include "lauxlib.h"

#include "error.h"
#include "platform.h"
#include "script.h"
#include "vfs.h"

static void test_serialize(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    int ok = luaL_dostring(L, "return {true, 1, 2.5, 'string', {123, 456}, {x=123, y=456}}");
    assert_true(ok == LUA_OK);

    buffer_t* serialized = script_to_serialized(L, -1);
    assert_true(error_count() == 0);
    assert_non_null(serialized);

    //fprintf(stderr, "size: %zu\ndata: ", serialized->size);
    //for (size_t i = 0;i < serialized->size;i++) {
    //    fprintf(stderr, "%02X ", serialized->data[i]);
    //}
    //fprintf(stderr, "\n");

    buffer_delete(serialized);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

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

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_serialize),
        cmocka_unit_test(test_wrap_cfuncs),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
