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
#include "ruleset.h"
#include "state.h"

typedef struct ingame_s {
    /**
     * The current in-use ruleset.
     * 
     * This is not an owning reference.  Do not free it.
     */
    ruleset_t* ruleset;

    /**
     * All current ingame state that exists outside of Lua.
     */
    state_t* state;
} ingame_t;

/**
 * Process ingame events
 */
static int ingame_frame(screen_t* screen, const gameevents_t* events) {
    ingame_t* ingame = screen->screen.ingame;
    if (state_frame(ingame->state) == false) {
        return RULESET_RESULT_ERROR;
    }
    ruleset_result_t res = ruleset_frame(ingame->ruleset, ingame->state, &events->game);
    return res;
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
static void ingame_render(screen_t* screen, render_module_t* render) {
    ingame_t* ingame = screen->screen.ingame;
    render->draw_state(ingame->state);
}

/**
 * Free ingame screen
 */
static void ingame_delete(screen_t* screen) {
    if (screen->screen.ingame != NULL) {
        if (screen->screen.ingame->state != NULL) {
            state_delete(screen->screen.ingame->state);
            screen->screen.ingame->state = NULL;
        }

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
screen_t ingame_new(ruleset_t* ruleset) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    ingame_t* ingame = calloc(1, sizeof(ingame_t));
    if (ingame == NULL) {
        return screen;
    }

    // Create our gamestate
    state_t* state = state_new(ruleset);
    if (state == NULL) {
        free(ingame);
        ingame = NULL;
        return screen;
    }

    ingame->state = state;
    ingame->ruleset = ruleset;

    screen.config = ingame_screen;
    screen.screen.ingame = ingame;
    return screen;
}