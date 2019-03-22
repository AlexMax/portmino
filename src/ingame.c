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
#include "environment.h"
#include "error.h"
#include "gametype.h"
#include "ruleset.h"

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
     * This _is_ an owning reference, which can be safely freed.
     */
    ruleset_t* ruleset;

    /**
     * The complete environment of the game we're playing.
     */
    environment_t* environment;
} ingame_t;

typedef enum {
    INGAME_RESULT_OK,
    INGAME_RESULT_ERROR,
    INGAME_RESULT_GAMEOVER
} playmenu_result_t;

/**
 * Process ingame inputs
 */
static int ingame_frame(screen_t* screen, const gameinputs_t* inputs) {
    ingame_t* ingame = screen->screen.ingame;
    if (environment_frame(ingame->environment, &inputs->game) == false) {
        return INGAME_RESULT_ERROR;
    }

    return INGAME_RESULT_OK;
}

/**
 * Navigate to the proper destination
 */
static void ingame_navigate(screens_t* screens, int result) {
    switch ((playmenu_result_t)result) {
    case INGAME_RESULT_OK:
        // Do nothing
        break;
    case INGAME_RESULT_ERROR:
        screens_pop(screens);
        break;
    case INGAME_RESULT_GAMEOVER:
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

    environment_draw(ingame->environment);
}

/**
 * Free ingame screen
 */
static void ingame_delete(screen_t* screen) {
    if (screen->screen.ingame != NULL) {
        gametype_delete(screen->screen.ingame->gametype);
        screen->screen.ingame->gametype = NULL;
        environment_delete(screen->screen.ingame->environment);
        screen->screen.ingame->environment = NULL;
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

    // Create our game environment
    environment_t* environment = environment_new(ruleset->lua, ruleset->name, gametype->name);
    if (environment == NULL) {
        free(ingame);
        return screen;
    }

    // Because we're about to be ingame, actually initialize the game
    if (environment_start(environment) == false) {
        environment_delete(environment);
        free(ingame);
        return screen;
    }

    ingame->environment = environment;
    ingame->ruleset = ruleset;
    ingame->gametype = gametype;

    screen.config = ingame_screen;
    screen.screen.ingame = ingame;
    return screen;
}
