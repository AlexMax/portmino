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

#include "define.h"

#include <stdlib.h>
#include <string.h>

#include "physfs.h"
#include "lualib.h"
#include "lauxlib.h"

#include "audioscript.h"
#include "board.h"
#include "boardscript.h"
#include "eventscript.h"
#include "frontend.h"
#include "globalscript.h"
#include "menu.h"
#include "nextscript.h"
#include "piecescript.h"
#include "randomscript.h"
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
ruleset_t* ruleset_new(const char* name) {
    char* rulesetname = strdup(name);
    if (rulesetname == NULL) {
        frontend_fatalerror("Allocation error.");
        return NULL;
    }

    char* filepath = NULL;
    int ok = asprintf(&filepath, "ruleset/%s/main.lua", name);
    if (ok < 0) {
        frontend_fatalerror("Allocation error.");
        return NULL;
    }

    // Load the file with our ruleset in it.
    buffer_t* file = vfs_file(filepath);
    free(filepath);
    filepath = NULL;
    if (file == NULL) {
        frontend_fatalerror("Could not find ruleset %s", name);
        return NULL;
    }

    // Create a new Lua state
    lua_State* L = luaL_newstate();
    if (L == NULL) {
        buffer_delete(file);
        return NULL;
    }

    static const luaL_Reg loadedlibs[] = {
        { "_G", globalscript_openlib },
        { "mino_ruleset", ruleset_openlib },
        { "mino_board", boardscript_openlib },
        { "mino_piece", piece_openlib },
        { "mino_audio", audio_openlib },
        { "mino_random", randomscript_openlib },
        { "mino_next", next_openlib },
        { "mino_event", eventscript_openlib },
        { NULL, NULL }
    };

    // Push our library functions into state.
    for (const luaL_Reg* lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    // Load our file into the newly-created Lua state.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, "main", "t") != LUA_OK) {
        buffer_delete(file);
        lua_close(L);
        frontend_fatalerror("Could not load ruleset %s", "default");
        return NULL;
    }
    buffer_delete(file);

    // Create a restricted ruleset environment and push a ref to it into
    // the registry.
    lua_createtable(L, 0, 0);
    lua_pushvalue(L, -1);
    int env_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pop env table dupe
    if (env_ref == LUA_REFNIL) {
        lua_close(L);
        frontend_fatalerror("Can't get a reference for \"env_ref\"");
        return NULL;
    }

    // Set up links to the proper modules and globals.
    const char* modules[] = {
        "mino_ruleset", "mino_board", "mino_piece", "mino_audio", "mino_random",
        "mino_next", "mino_event"
    };
    lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
    for (size_t i = 0;i < sizeof(modules) / sizeof(modules[0]);i++) {
        lua_getfield(L, -1, modules[i]);
        lua_setfield(L, -3, modules[i]);
    }
    lua_pop(L, 1);

    const char* globals[] = {
        "ipairs", "next", "pairs", "print", "require", "tostring", "_VERSION"
    };
    for (size_t i = 0;i < sizeof(globals) / sizeof(globals[0]);i++) {
        lua_getglobal(L, globals[i]);
        lua_setfield(L, -2, globals[i]);
    }

    // Setup the global "package.loaded" table we use for packages.
    lua_createtable(L, 0, 1); // package table
    lua_createtable(L, 0, 0); // loaded table
    lua_setfield(L, -2, "loaded"); // pops loaded table
    lua_setfield(L, -2, "package"); // pops package table

    // Set our environment context, both in the registry and as our _ENV.
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "env"); // pop env copy
    lua_setupvalue(L, -2, 1); // pop env

    // We now have the ruleset on the top of the stack.  Call it!
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1); // Error message
        frontend_fatalerror("Could not execute ruleset %s\n%s", "default", err);
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

    ruleset->name = rulesetname;
    ruleset->lua = L;
    ruleset->state_frame_ref = state_frame_ref;
    ruleset->state_ref = state_ref;
    ruleset->pieces = piece_configs;
    ruleset->next_piece_ref = next_piece_ref;
    ruleset->env_ref = env_ref;

    return ruleset;
}

/**
 * Free a ruleset.
 */
void ruleset_delete(ruleset_t* ruleset) {
    if (ruleset->name != NULL) {
        free(ruleset->name);
        ruleset->name = NULL;
    }

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

/**
 * Get a list of gametypes that can be played in a given ruleset
 * 
 * This function allocates a menulist.  It's up to the caller to free it.
 */
menulist_t* ruleset_get_gametypes(ruleset_t* ruleset) {
    char* gametypepath = NULL;
    int ok = asprintf(&gametypepath, "gametype/%s", ruleset->name);
    if (ok < 0) {
        // Allocation error
        goto fail;
    }

    menulist_t* list = menulist_new();
    if (list == NULL) {
        // Allocation error
        goto fail;
    }

    // Iterate over all of the ruleset gametype directories
    char** gametypes = PHYSFS_enumerateFiles(gametypepath);
    free(gametypepath);
    gametypepath = NULL;
    if (gametypes == NULL) {
        // Allocation error
        goto fail;
    }
    for (char** i = gametypes;*i != NULL;i++) {
        // Try and find a file called main.lua in every directory
        char* mainpath = NULL;
        ok = asprintf(&mainpath, "gametype/%s/%s/main.lua", ruleset->name, *i);
        if (ok < 0) {
            // Allocation error.
            goto fail;
        }

        PHYSFS_Stat stat;
        ok = PHYSFS_stat(mainpath, &stat);
        free(mainpath);
        if (ok == 0) {
            // File does not exist.
            continue;
        }
        if (stat.filetype != PHYSFS_FILETYPE_REGULAR) {
            // File isn't a file.
            continue;
        }

        menulist_params_t params;
        params.value = *i;
        params.label = *i;
        params.help = "help";
        params.position = 10;
        menulist_push(list, &params);
    }
    PHYSFS_freeList(gametypes);
    gametypes = NULL;


    return list;

fail:
    if (gametypepath != NULL) {
        free(gametypepath);
        gametypepath = NULL;
    }

    if (list != NULL) {
        menulist_delete(list);
        list = NULL;
    }

    if (gametypes != NULL) {
        PHYSFS_freeList(gametypes);
        gametypes = NULL;
    }

    return NULL;
}

ruleset_result_t ruleset_frame(ruleset_t* ruleset, state_t* state,
                               const playerevents_t* playerevents) {
    // Push our context into the registry so we can get at them from C
    // functions called from inside Lua.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->env_ref);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "env");
    lua_pushlightuserdata(ruleset->lua, ruleset);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "ruleset");
    lua_pushlightuserdata(ruleset->lua, state);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "state");
    lua_pushlightuserdata(ruleset->lua, (void*)playerevents);
    lua_setfield(ruleset->lua, LUA_REGISTRYINDEX, "playerevents");

    // Use our references to grab the state_frame function and its environment
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->state_frame_ref);
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->env_ref);

    // Setup the environment
    lua_setupvalue(ruleset->lua, -2, 1);

    // Run the state_frame function.
    if (lua_pcall(ruleset->lua, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return RULESET_RESULT_ERROR;
    }

    // Handle our result
    ruleset_result_t result = lua_tointeger(ruleset->lua, -1);
    lua_pop(ruleset->lua, 1); // pop result

    return result;
}

/**
 * Return a "next" piece by calling into our Lua function to get it
 */
const piece_config_t* ruleset_next_piece(ruleset_t* ruleset, next_t* next) {
    // Use our reference to grab the next_piece function.
    lua_rawgeti(ruleset->lua, LUA_REGISTRYINDEX, ruleset->next_piece_ref);

    lua_pushstring(ruleset->lua, "ruleset");
    lua_pushlightuserdata(ruleset->lua, (void*)ruleset);
    lua_settable(ruleset->lua, LUA_REGISTRYINDEX);

    // Pass a 1-indexed board ID to the next piece function, so we know which
    // board we're spawning the piece on.
    lua_pushinteger(ruleset->lua, next->id + 1);

    // Call it, friendo
    if (lua_pcall(ruleset->lua, 1, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(ruleset->lua, -1);
        frontend_fatalerror("lua error: %s", err);
        return NULL;
    }

    // Return either our config pointer or NULL
    return lua_touserdata(ruleset->lua, -1);
}
