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

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "boardscript.h"
#include "frontend.h"
#include "piecescript.h"
#include "ruleset.h"
#include "script.h"
#include "state.h"
#include "vfs.h"

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
static int ruleset_openlib(lua_State* L) {
    static const luaL_Reg rulesetlib[] = {
        { "get_gametic", ruleset_get_gametic },
        { "get_player_events", ruleset_get_player_events },
        { "get_piece_configs", ruleset_get_piece_configs },
        { NULL, NULL }
    };

    luaL_newlib(L, rulesetlib);

    return 1;
}

/**
 * Allocate a new ruleset.
 */
ruleset_t* ruleset_new(void) {
    // Load the file with our ruleset in it.
    buffer_t* file = vfs_file("ruleset/default/main.lua");
    if (file == NULL) {
        frontend_fatalerror("Could not find ruleset %s", "default");
        return NULL;
    }

    // Create a new Lua state
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        buffer_delete(file);
        return NULL;
    }

    static const luaL_Reg loadedlibs[] = {
        { "_G", luaopen_base },
        { "mino_ruleset", ruleset_openlib },
        { "mino_board", boardscript_openlib },
        { "mino_piece", piece_openlib },
        { NULL, NULL }
    };

    // Push our library functions into state.
    for (const luaL_Reg* lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    // Load our file into the newly-created Lua state.
    if (luaL_loadbuffer(L, (char*)file->data, file->size, "main") != LUA_OK) {
        buffer_delete(file);
        lua_close(L);
        frontend_fatalerror("Could not load ruleset %s", "default");
        return NULL;
    }
    buffer_delete(file);

    // We now have the ruleset on the top of the stack.  Call it!
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        lua_close(L);
        frontend_fatalerror("Could not execute ruleset %s", "default");
        return NULL;
    }

    // We should have a table at the top of the stack that contains the
    // functions that implement the ruleset.
    if (lua_type(L, -1) != LUA_TTABLE) {
        lua_close(L);
        frontend_fatalerror("Could not find module table in ruleset %s", "default");
        return NULL;
    }

    // Check for a table key that contains the state_frame function.
    lua_pushstring(L, "state_frame");
    int type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        lua_close(L);
        frontend_fatalerror("Could not find function \"state_frame\" in module table in ruleset %s", "default");
        return NULL;
    } else if (type != LUA_TFUNCTION) {
        lua_close(L);
        frontend_fatalerror("\"state_frame\" is not a function in ruleset %s", "default");
        return NULL;
    }

    // Create a reference to the state_frame function.
    int state_frame_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (state_frame_ref == LUA_REFNIL) {
        lua_close(L);
        frontend_fatalerror("Can't get a reference for \"state_frame\"");
        return NULL;
    }

    // Check for a table key that contains the state table.
    lua_pushstring(L, "state");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        lua_close(L);
        frontend_fatalerror("Could not find table \"state\" in module table in ruleset %s", "default");
        return NULL;
    } else if (type != LUA_TTABLE) {
        lua_close(L);
        frontend_fatalerror("\"state\" is not a table in ruleset %s", "default");
        return NULL;
    }

    // Create a reference to the state table.
    int state_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (state_ref == LUA_REFNIL) {
        lua_close(L);
        frontend_fatalerror("Can't get a reference for \"state\"");
        return NULL;
    }

    // Load our pieces.
    lua_pushstring(L, "pieces");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        lua_close(L);
        frontend_fatalerror("Could not find table \"pieces\" in module table in ruleset %s", "default");
        return NULL;
    } else if (type != LUA_TTABLE) {
        lua_close(L);
        frontend_fatalerror("\"pieces\" is not a table in ruleset %s", "default");
        return NULL;
    }

    // Iterate through the piece array.
    piece_configs_t* piece_configs = piece_configs_new(L); // pops piece array
    if (piece_configs == NULL) {
        // FIXME: Figure out how to close Lua state safely.
        const char* error = lua_tostring(L, -1);
        frontend_fatalerror("Error parsing pieces in ruleset %s: %s", "default", error);
        return NULL;
    }

    // Check for a table key that contains the next_piece function.
    lua_pushstring(L, "next_piece");
    type = lua_gettable(L, -2);
    if (type == LUA_TNIL) {
        lua_close(L);
        frontend_fatalerror("Could not find function \"next_piece\" in module table in ruleset %s", "default");
        return NULL;
    } else if (type != LUA_TFUNCTION) {
        lua_close(L);
        frontend_fatalerror("\"next_piece\" is not a function in ruleset %s", "default");
        return NULL;
    }

    // Create a reference to the next_piece function.
    int next_piece_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (next_piece_ref == LUA_REFNIL) {
        lua_close(L);
        frontend_fatalerror("Can't get a reference for \"next_piece\"");
        return NULL;
    }

    // We should only have the ruleset on the stack.  Always finish your
    // Lua meddling with a clean stack.
    if (lua_gettop(L) != 1) {
        lua_close(L);
        frontend_fatalerror("Lua stack not cleaned up in ruleset_new");
        return NULL;
    }
    lua_pop(L, 1);

    // We're all set!  Create the structure to actually hold our ruleset.
    ruleset_t* ruleset = calloc(1, sizeof(ruleset_t));
    if (ruleset == NULL) {
        return NULL;
    }

    ruleset->lua = L;
    ruleset->state_frame_ref = state_frame_ref;
    ruleset->state_ref = state_ref;
    ruleset->pieces = piece_configs;
    ruleset->next_piece_ref = next_piece_ref;

    return ruleset;
}

/**
 * Free a ruleset.
 */
void ruleset_delete(ruleset_t* ruleset) {
    if (ruleset->lua != NULL) {
        lua_close(ruleset->lua);
        ruleset->lua = NULL;
    }

    if (ruleset->pieces != NULL) {
        piece_configs_delete(ruleset->pieces);
        ruleset->pieces = NULL;
    }

    free(ruleset);
}

ruleset_result_t ruleset_frame(ruleset_t* ruleset, state_t* state,
                               const playerevents_t* playerevents) {
    // Push our state and playerevents into the registry so we can get at
    // them from C functions called from inside Lua.
    lua_pushstring(ruleset->lua, "state");
    lua_pushlightuserdata(ruleset->lua, state);
    lua_settable(ruleset->lua, LUA_REGISTRYINDEX);
    lua_pushstring(ruleset->lua, "playerevents");
    lua_pushlightuserdata(ruleset->lua, (void*)playerevents);
    lua_settable(ruleset->lua, LUA_REGISTRYINDEX);

    // Use our reference to grab the state_frame function.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->state_frame_ref);

    // We expect the top of the stack to contain a function that contains
    // our state_frame function.
    if (lua_pcall(ruleset->lua, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return RULESET_RESULT_ERROR;
    }

    return RULESET_RESULT_OK;
}

/**
 * Return a "next" piece by calling into our Lua function to get it
 */
const piece_config_t* ruleset_next_piece(ruleset_t* ruleset) {
    // Use our reference to grab the next_piece function.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->next_piece_ref);

    lua_pushstring(ruleset->lua, "ruleset");
    lua_pushlightuserdata(ruleset->lua, (void*)ruleset);
    lua_settable(ruleset->lua, LUA_REGISTRYINDEX);

    // Call it, friendo
    if (lua_pcall(ruleset->lua, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return NULL;
    }

    // Return either our config pointer or NULL
    return lua_touserdata(ruleset->lua, -1);
}
