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

#include "rulesetscript.h"

#include "lua.h"
#include "lauxlib.h"

#include "ruleset.h"
#include "state.h"

/**
 * Lua: Get the current gametic of the state
 */
static int ruleset_get_gametic(lua_State* L) {
    lua_pushstring(L, "state");
    lua_gettable(L, LUA_REGISTRYINDEX);

    const state_t* state = lua_touserdata(L, -1);
    if (state == NULL) {
        // never returns
        luaL_error(L, "ruleset_get_gametic is missing internal state");
    }

    lua_pushnumber(L, state->tic);
    return 1;
}

/**
 * Lua: Grabs player events for a specific player
 */
static int ruleset_get_player_events(lua_State* L) {
    // Parameter 1: Player number, 1-indexed
    lua_Integer player = luaL_checkinteger(L, 1);
    if (player <= 0 || player >= MINO_MAX_PLAYERS) {
         // never returns
         luaL_argerror(L, 1, "invalid player id");
    }
    player -= 1;

    lua_pushstring(L, "playerevents");
    lua_gettable(L, LUA_REGISTRYINDEX);

    const playerevents_t* playerevents = lua_touserdata(L, -1);
    if (playerevents == NULL) {
        // never returns
        luaL_error(L, "ruleset_get_player_events is missing internal state");
    }

    // Result: Player events bitfield.
    lua_pushinteger(L, playerevents->events[player]);
    return 1;
}

/**
 * Lua: Return a list of piece configs attached to the state.
 */
static int ruleset_get_piece_configs(lua_State* L) {
    lua_pushstring(L, "ruleset");
    lua_gettable(L, LUA_REGISTRYINDEX);

    const ruleset_t* ruleset = lua_touserdata(L, -1);
    if (ruleset == NULL) {
        // never returns
        luaL_error(L, "ruleset_get_piece_configs is missing internal state");
    }

    // Fetch the pieces and return an array of configs as a table.
    lua_createtable(L, ruleset->pieces->size, 0);
    for (size_t i = 1;i <= ruleset->pieces->size;i++) {
        // Lua is 1-indexed, but C is 0.
        lua_pushlightuserdata(L, ruleset->pieces->configs[i - 1]);
        lua_rawseti(L, -2, i);
    }

    return 1;
}

/**
 * Push library functions into the state.
 */
int rulesetscript_openlib(lua_State* L) {
    static const luaL_Reg rulesetlib[] = {
        { "get_gametic", ruleset_get_gametic },
        { "get_player_events", ruleset_get_player_events },
        { "get_piece_configs", ruleset_get_piece_configs },
        { NULL, NULL }
    };

    luaL_newlib(L, rulesetlib);

    return 1;
}
