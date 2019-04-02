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
#include "piece.h"
#include "script.h"

/**
 * Create a environment that our game scripts can run inside
 */
environment_t* environment_new(lua_State* L, const char* ruleset, const char* gametype) {
    environment_t* env = NULL;
    int top = lua_gettop(L);

    // Allocate our scripting environment
    if ((env = calloc(1, sizeof(environment_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    env->lua = L;
    env->registry_ref = LUA_NOREF;
    env->env_ref = LUA_NOREF;
    env->ruleset_ref = LUA_NOREF;
    env->state_ref = LUA_NOREF;
    env->gametic = 0;
    env->pieces = NULL;

    // Create an environment-specific registry table and push a ref to it
    // into the global registry.
    lua_newtable(L);
    lua_pushvalue(L, -1);
    if ((env->env_ref = luaL_ref(L, LUA_REGISTRYINDEX)) == LUA_REFNIL) { // pop registry table dupe
        error_push_allocerr();
        goto fail;
    }

    // We need to populate our registry with a few things...
    script_push_paths(L, ruleset, gametype); // paths
    lua_setfield(L, -2, "paths");
    script_push_cpaths(L, ruleset, gametype); // config paths
    lua_setfield(L, -2, "config_paths");
    lua_newtable(L); // loaded
    lua_setfield(L, -2, "loaded");

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
        "mino_audio", "mino_board", "mino_input", "mino_piece", "mino_random",
        "mino_render"
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

void environment_delete(environment_t* env) {
    if (env == NULL) {
        return;
    }

    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->state_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->env_ref);
    luaL_unref(env->lua, LUA_REGISTRYINDEX, env->registry_ref);

    piece_configs_delete(env->pieces);
    env->pieces = NULL;

    free(env);
}

/**
 * Initialize our script environment for a new game
 */
bool environment_start(environment_t* env) {
    int top = lua_gettop(env->lua);

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

    if (lua_pcall(env->lua, 1, 1, 0) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

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

    // Try and call a function called "frame" to advance the game.
    if (lua_rawgeti(env->lua, LUA_REGISTRYINDEX, env->ruleset_ref) != LUA_TTABLE) {
        error_push("Ruleset module reference has gone stale.");
        goto fail;
    }

    if (lua_getfield(env->lua, -1, "frame") != LUA_TFUNCTION) {
        error_push("Ruleset module has no start function.");
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
    lua_pushlightuserdata(env->lua, (void*)inputs);

    if (lua_pcall(env->lua, 3, 1, 0) != LUA_OK) {
        error_push("lua_error: %s", lua_tostring(env->lua, -1));
        goto fail;
    }

    env->gametic += gametic;
    return true;

fail:
    lua_settop(env->lua, top);
    return false;
}

/**
 * Draw the current frame of game logic
 */
void environment_draw(environment_t* env) {
    (void)env;
}
