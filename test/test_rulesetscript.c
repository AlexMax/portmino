#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "basemino.h"
#include "error.h"
#include "frontend.h"
#include "game.h"
#include "gametype.h"
#include "platform.h"
#include "ruleset.h"
#include "script.h"
#include "vfs.h"

buffer_t g_basemino;

static buffer_t* test_basemino(void) {
    if (g_basemino.data == NULL) {
        g_basemino.data = basemino_pk3;
        g_basemino.size = basemino_pk3_len;
    }
    return &g_basemino;
}

static void test_fatalerror(const char *fmt, va_list va) {
    fail_msg("fatalerror triggered\n");
}

static void test_gametype_call(void** state) {
    (void)state;

    frontend_module_t module = {
        test_basemino,
        test_fatalerror,
    };
    frontend_init(&module);
    platform_init();
    vfs_init(NULL);

    buffer_t script;
    script.data = (uint8_t*)
        "local function foo(p1, p2)\n"
        "    return 'FOO', 'BAR'\n"
        "end\n"
        "return { state_functions = { foo = foo } }";
    script.size = strlen((char*)script.data);

    lua_State* L = script_newstate();
    assert_non_null(L);
    ruleset_t* ruleset = ruleset_new(L, "default");
    assert_non_null(ruleset);
    gametype_t* gametype = gametype_new(L, &script, "test");
    assert_non_null(gametype);

    lua_pushlightuserdata(L, gametype);
    lua_setfield(L, LUA_REGISTRYINDEX, "gametype");

    // Test our sample gametype function "foo".
    luaL_dostring(L, "return mino_ruleset.gametype_call('foo', 'bar', 'baz')");
    assert_true(lua_gettop(L) == 2);
    assert_string_equal(lua_tostring(L, 1), "FOO");
    assert_string_equal(lua_tostring(L, 2), "BAR");
    lua_pop(L, 2);

    gametype_delete(gametype);
    ruleset_delete(ruleset);
    lua_close(L);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_gametype_call),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
