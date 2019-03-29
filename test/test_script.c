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

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_serialize),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
