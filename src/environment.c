/**
 * This file is part of Portmino.
 *
 * Portmino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Portmino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Portmino.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "environment.h"

#include <stdlib.h>

#include "lauxlib.h"

#include "error.h"
#include "inputscript.h"
#include "proto.h"
#include "script.h"

static lua_State *getthread (lua_State *L, int *arg) {
    if (lua_isthread(L, 1)) {
        *arg = 1;
        return lua_tothread(L, 1);
    }
    else {
        *arg = 0;
        return L;  /* function will operate over current thread */
    }
}

/**
 * debug.backtrace from Lua stdlib
 */
static int db_traceback(lua_State *L) {
    int arg;
    lua_State *L1 = getthread(L, &arg);
    const char *msg = lua_tostring(L, arg + 1);
    if (msg == NULL && !lua_isnoneornil(L, arg + 1))  /* non-string 'msg'? */
        lua_pushvalue(L, arg + 1);  /* return it untouched */
    else {
        int level = (int)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
        luaL_traceback(L, L1, msg, level);
    }
    return 1;
}

/**
 * Create a environment that our game scripts can run inside
 */
environment_t* environment_new(lua_State* L, const char* ruleset, const char* gametype) {
    environment_t* env = NULL;
    proto_container_t* protos = NULL;
    int top = lua_gettop(L);

    // Allocate our scripting environment
    if ((env = calloc(1, sizeof(environment_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Allocate our prototype container
    if ((protos = proto_container_new()) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    env->lua = L;
    env->registry_ref = LUA_NOREF;
    env->env_ref = LUA_NOREF;
    env->ruleset_ref = LUA_NOREF;
    env->state_ref = LUA_NOREF;
    env->gametic = 0;
    env->protos = protos;
    for (size_t i;i < ARRAY_LEN(env->states);i++) {
        env->states[i] = NULL;
    }

    // Create an environment-specific registry table and push a ref to it
    // into the global registry.
    lua_newtable(L);
    lua_pushvalue(L, -1);
    if ((env->registry_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop registry table dupe
        error_push_allocerr();
        goto fail;
    }

    // We need to populate our registry with a few things...
    lua_pushinteger(L, env->registry_ref); // registry reference
    lua_setfield(L, -2, "registry_ref");
    script_push_paths(L, ruleset, gametype); // paths
    lua_setfield(L, -2, "paths");
    script_push_cpaths(L, ruleset, gametype); // config paths
    lua_setfield(L, -2, "config_paths");
    lua_newtable(L); // loaded
    lua_setfield(L, -2, "loaded");
    lua_pushlightuserdata(L, env->protos);
    lua_setfield(L, -2, "proto_container");
    lua_newtable(L); // prototype lookup table
    lua_setfield(L, -2, "proto_hash");
    lua_pushinteger(L, 1); // next entity id
    lua_setfield(L, -2, "entity_next");

    // Create a restricted ruleset environment and push a ref to it into
    // the registry, plus add it to the registry.
    lua_newtable(L);
    lua_pushvalue(L, -1);
    if ((env->env_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop env table dupe
        error_push_allocerr();
        goto fail;
    }
    // ..and again, but this time put a reference to it in the local registry
    // as well.
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "_ENV");

    // For our new environment, set up links to the proper modules and globals.
    const char* modules[] = {
        "mino_audio", "mino_board", "mino_input", "mino_piece", "mino_proto",
        "mino_random", "mino_render", "string", "table"
    };
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    for (size_t i = 0;i < ARRAY_LEN(modules);i++) {
        lua_getfield(L, -1, modules[i]);
        script_wrap_cfuncs(L, -4);
        lua_setfield(L, -3, modules[i]);
    }
    lua_pop(L, 1);

    const char* globals[] = {
        "doconfig", "ipairs", "next", "pairs", "print", "require",
        "tostring", "_VERSION"
    };
    for (size_t i = 0;i < ARRAY_LEN(globals);i++) {
        lua_getglobal(L, globals[i]);
        script_wrap_cfuncs(L, -3);
        lua_setfield(L, -2, globals[i]);
    }

    // Call require('ruleset')! 
    lua_getfield(L, -1, "require");
    lua_pushstring(L, "ruleset");
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(L, -1));
        goto fail;
    }

    // Keep a reference to our ruleset module table.
    if ((env->ruleset_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop ruleset table
        error_push_allocerr();
        goto fail;
    }

    // Create a state table and push a ref to it into the registry.
    lua_newtable(L);
    if ((env->state_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop env table dupe
        error_push_allocerr();
        goto fail;
    }

    // Always finish your Lua meddling with a clean stack.
    lua_pop(L, 2);
    if (lua_gettop(L) != top) {
        error_push("lua stack not clear");
        goto fail;
    }

    return env;

fail:
    environment_delete(env);
    return NULL;
}

/**
 * Destroy the environment
 */
void environment_delete(environment_t* env) {
    if (env == NULL) {
        return;
    }

    for (size_t i = 0;i < ARRAY_LEN(env->states);i++) {
        if (env->states[i] != NULL) {
            buffer_delete(env->states[i]);
            env->states[i] = NULL;
        }
    }

    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->state_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->env_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->registry_ref);

    proto_container_delete(env->protos);
    env->protos = NULL;

    free(env);
}

/**
 * Run an arbitrary Lua script within the context of our environment
 *
 * This function is usually reserved for our unit tests.  There shouldn't
 * be any reason to use it otherwise.
 */
bool environment_dostring(environment_t* env, const char* script) {
    int top = lua_gettop(env->lua);

    // Load the string into a chunk
    if (luaL_loadstring(env->lua, script) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    // Load our environment and set the environment of the chunk to it
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->env_ref) != LUA_TTABLE) {
        error_push("Environment reference has gone stale.");
        goto fail;
    }
    lua_setupvalue(env->lua, -2, 1);

    // Call the chunk
    if (lua_pcall(env->lua, 0, LUA_MULTRET, 0) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    return true;

fail:
    lua_settop(env->lua, top);
    return false;
}

/**
 * Initialize our script environment for a new game
 */
bool environment_start(environment_t* env) {
    int top = lua_gettop(env->lua);

    // Error message handler
    lua_pushcfunction(env->lua, db_traceback);

    // Try and call a function called "start" to initialize the game.
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref) != LUA_TTABLE) {
        error_push("Ruleset module reference has gone stale.");
        goto fail;
    }

    if (lua_getfield(env->lua, -1, "start") != LUA_TFUNCTION) {
        error_push("Ruleset module has no start function.");
        goto fail;
    }

    // Parameter 1: State table
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->state_ref) != LUA_TTABLE) {
        error_push("State table reference has gone stale.");
        goto fail;
    }

    if (lua_pcall(env->lua, 1, 1, top + 1) != LUA_OK) {
        error_push("Lua error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    // Serialize our intial state
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->state_ref) != LUA_TTABLE) {
        error_push("State table reference has gone stale.");
        goto fail;
    }

    buffer_t* state = script_to_serialized(env->lua, -1);
    if (state == NULL) {
        error_push("Initial state could not be serialized.");
        goto fail;
    }
    env->states[0] = state;

    lua_settop(env->lua, top);
    return true;

fail:
    lua_settop(env->lua, top);
    return false;
}

/**
 * Rewind the environment to a specific past frame
 */
bool environment_rewind(environment_t* env, uint32_t frame) {
    (void)env;
    (void)frame;

    return true;
}

/**
 * Run one frame worth of game logic
 */
bool environment_frame(environment_t* env, const playerinputs_t* inputs) {
    int top = lua_gettop(env->lua);

    // Error message handler
    lua_pushcfunction(env->lua, db_traceback);

    // Try and call a function called "frame" to advance the game.
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref) != LUA_TTABLE) {
        error_push("Ruleset module reference has gone stale.");
        goto fail;
    }

    if (lua_getfield(env->lua, -1, "frame") != LUA_TFUNCTION) {
        error_push("Ruleset module has no frame function.");
        goto fail;
    }

    // Parameter 1: State table
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->state_ref) != LUA_TTABLE) {
        error_push("State table reference has gone stale.");
        goto fail;
    }

    // Parameter 2: Gametic
    uint32_t gametic = env->gametic + 1;
    lua_pushinteger(env->lua, gametic);

    // Parameter 3: Player inputs
    inputscript_push_inputs(env->lua, inputs);

    if (lua_pcall(env->lua, 3, 1, top + 1) != LUA_OK) {
        error_push("Lua error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    env->gametic += 1;
    lua_settop(env->lua, top);
    return true;

fail:
    lua_settop(env->lua, top);
    return false;
}

/**
 * Draw the current frame of game logic
 */
void environment_draw(environment_t* env) {
    int top = lua_gettop(env->lua);

    // Error message handler
    lua_pushcfunction(env->lua, db_traceback);

    // Try and call a function called "draw" to draw the game.
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref) != LUA_TTABLE) {
        error_push("Ruleset module reference has gone stale.");
        goto fail;
    }

    if (lua_getfield(env->lua, -1, "draw") != LUA_TFUNCTION) {
        error_push("Ruleset module has no draw function.");
        goto fail;
    }

    // Parameter 1: State table
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->state_ref) != LUA_TTABLE) {
        error_push("State table reference has gone stale.");
        goto fail;
    }

    if (lua_pcall(env->lua, 1, 1, top + 1) != LUA_OK) {
        error_push("Lua error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    lua_settop(env->lua, top);
    return;

fail:
    lua_settop(env->lua, top);
}
