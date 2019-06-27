#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"
#include "lauxlib.h"

#include "platform.h"
#include "script.h"
#include "serialize.h"
#include "vfs.h"

static void test_serialize(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    int ok = luaL_dostring(L, "return {true, 1, 2.5, 'string', {123, 456}, {x=123, y=456}}");
    assert_true(ok == LUA_OK);

    buffer_t* serialized = serialize_to_serialized(L, -1);
    assert_true(error_count() == 0);
    assert_non_null(serialized);

    fprintf(stderr, "size: %zu\ndata: ", serialized->size);
    for (size_t i = 0; i < serialized->size; i++) {
        fprintf(stderr, "%02X ", serialized->data[i]);
    }
    fprintf(stderr, "\n");

    lua_newtable(L); // push table
    int ref = luaL_ref(L, LUA_REGISTRYINDEX); // pop table

    serialize_push_serialized(L, ref, serialized);
    assert_true(error_count() == 0);

    buffer_delete(serialized);

    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_serialize),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
