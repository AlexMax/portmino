#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "test.h"

#include "lua.h"

#include "environment.h"
#include "piece.h"
#include "platform.h"
#include "proto.h"
#include "script.h"
#include "vfs.h"

static void test_protoscript_load(void** state) {
    frontend_init(&g_frontend_module);
    platform_init();
    assert_true(vfs_init(NULL) == true);

    lua_State* L = script_newstate();
    assert_non_null(L);

    environment_t* env = environment_new(L, "stdmino", "endurance");
    assert_non_null(env);

    // Does a proper piece work?
    bool ok = environment_dostring(env, "mino_proto.load('piece', 'test', {"
        "data = { 3, 3, 0, 0, 3, 3, 0, 0, 0,"
                 "0, 0, 3, 0, 3, 3, 0, 3, 0,"
                 "0, 0, 0, 3, 3, 0, 0, 3, 3,"
                 "0, 3, 0, 3, 3, 0, 3, 0, 0 },"
        "spawn_pos = { x = 3, y = 1 }, spawn_rot = 0, width = 3, height = 3 })");
    assert_true(ok == true);

    // Does a duplicate error out?
    ok = environment_dostring(env, "mino_proto.load('piece', 'test', {"
        "data = { 3, 3, 0, 0, 3, 3, 0, 0, 0,"
                 "0, 0, 3, 0, 3, 3, 0, 3, 0,"
                 "0, 0, 0, 3, 3, 0, 0, 3, 3,"
                 "0, 3, 0, 3, 3, 0, 3, 0, 0 },"
        "spawn_pos = { x = 3, y = 1 }, spawn_rot = 0, width = 3, height = 3 })");
    assert_true(ok == false);

    // Does an improper piece error out cleanly?
    ok = environment_dostring(env, "mino_proto.load('piece', 'empty', {})");
    assert_true(ok == false);

    // Does the piece we push into the protype container actually work?
    lua_rawgeti(L, LUA_REGISTRYINDEX, env->registry_ref);
    lua_getfield(L, -1, "proto_hash");
    lua_getfield(L, -1, "test");
    proto_t* proto = lua_touserdata(L, -1);
    assert_true(proto != NULL);
    assert_true(proto->type == MINO_PROTO_PIECE);
    piece_config_t* piece = proto->data;
    assert_true(piece != NULL);
    assert_true(piece->spawn_pos.x == 3);
    assert_true(piece->spawn_pos.y == 1);

    environment_delete(env);
    lua_close(L);

    vfs_deinit();
    platform_deinit();
    frontend_deinit();
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_protoscript_load),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
