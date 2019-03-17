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

#include "ingame.h"

#include <stdlib.h>

#include "audio.h"
#include "error.h"
#include "gametype.h"
#include "ruleset.h"
#include "state.h"

typedef struct ingame_s {
    /**
     * The current in-use gametype.
     * 
     * This is not an owning reference.  Do not free it.
     */
    gametype_t* gametype;

    /**
     * The current in-use ruleset.
     * 
     * This _is_ an owning reference, even though it's passed to the allocation
     * function, and must be freed.
     */
    ruleset_t* ruleset;

    /**
     * All current ingame state that exists outside of Lua.
     */
    state_t* state;
} ingame_t;

/**
 * Process ingame inputs
 */
static int ingame_frame(screen_t* screen, const gameinputs_t* inputs) {
    ingame_t* ingame = screen->screen.ingame;
    if (state_frame(ingame->state) == false) {
        return RULESET_RESULT_ERROR;
    }

    lua_State* L = ingame->ruleset->lua;

    // Push our context into the registry so we can get at them from C
    // functions called from inside Lua.
    /*lua_rawgeti(L, LUA_REGISTRYINDEX, ingame->ruleset->env_ref);
    lua_setfield(L, LUA_REGISTRYINDEX, "env");
    lua_pushlightuserdata(L, ingame->ruleset);
    lua_setfield(L, LUA_REGISTRYINDEX, "ruleset");
    lua_pushlightuserdata(L, ingame->gametype);
    lua_setfield(L, LUA_REGISTRYINDEX, "gametype");
    lua_pushlightuserdata(L, ingame->state);
    lua_setfield(L, LUA_REGISTRYINDEX, "state");
    lua_pushlightuserdata(L, (void*)(&inputs->game));
    lua_setfield(L, LUA_REGISTRYINDEX, "playerinputs");

    // Use our references to grab the state_frame function and its environment
    lua_rawgeti(L, LUA_REGISTRYINDEX, ingame->ruleset->state_frame_ref);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ingame->ruleset->env_ref);*/

    // Setup the environment
    lua_setupvalue(L, -2, 1);

    // Run the state_frame function.
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        error_push("lua error: %s", err);
        return RULESET_RESULT_ERROR;
    }

    // Handle our result
    ruleset_result_t result = lua_tointeger(L, -1);
    lua_pop(L, 1); // pop result

    return result;
}

/**
 * Navigate to the proper destination
 */
static void ingame_navigate(screens_t* screens, int result) {
    switch ((ruleset_result_t)result) {
    case RULESET_RESULT_OK:
        // Should never get here.
        break;
    case RULESET_RESULT_ERROR:
        screens_pop(screens);
        break;
    case RULESET_RESULT_TOPOUT:
        screens_pop(screens);
        audio_playsound(g_sound_gameover);
        break;
    }
}

/**
 * Render the ingame screen
 */
static void ingame_render(screen_t* screen) {
    ingame_t* ingame = screen->screen.ingame;
    lua_State* L = ingame->ruleset->lua;

    // Use our references to grab the draw function
    lua_rawgeti(L, LUA_REGISTRYINDEX, ingame->gametype->draw_ref);

    // Run the draw function.
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        error_push("lua error: %s", err);
    }
}

/**
 * Free ingame screen
 */
static void ingame_delete(screen_t* screen) {
    if (screen->screen.ingame != NULL) {
        gametype_delete(screen->screen.ingame->gametype);
        screen->screen.ingame->gametype = NULL;
        state_delete(screen->screen.ingame->state);
        screen->screen.ingame->state = NULL;
        free(screen->screen.ingame);
        screen->screen.ingame = NULL;
    }
}

screen_config_t ingame_screen = {
    SCREEN_INGAME,
    ingame_frame,
    ingame_navigate,
    ingame_render,
    ingame_delete
};

/**
 * Allocate ingame screen
 */
screen_t ingame_new(ruleset_t* ruleset, gametype_t* gametype) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    ingame_t* ingame = calloc(1, sizeof(ingame_t));
    if (ingame == NULL) {
        return screen;
    }

    // Create our gamestate
    state_t* state = state_new(ruleset, gametype);
    if (state == NULL) {
        free(ingame);
        return screen;
    }

    // Because we're about to be ingame, actually initialize the game
    if (state_initgame(state) == false) {
        state_delete(state);
        free(ingame);
        return screen;
    }

    ingame->state = state;
    ingame->ruleset = ruleset;
    ingame->gametype = gametype;

    screen.config = ingame_screen;
    screen.screen.ingame = ingame;
    return screen;
}
