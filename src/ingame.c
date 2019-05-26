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
#include "pausemenu.h"
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

    /**
     * Countdown until starting game, in frames.
     */
    int countdown;

    /**
     * Are we in the "game over" state?
     */
    bool gameover;
} ingame_t;

typedef enum {
    INGAME_RESULT_OK,
    INGAME_RESULT_ERROR,
    INGAME_RESULT_GAMEOVER,
    INGAME_RESULT_PAUSE,
} playmenu_result_t;

/**
 * Process ingame inputs
 */
static int ingame_frame(screen_t* screen, const gameinputs_t* inputs) {
    ingame_t* ingame = screen->screen.ingame;

    if (ingame->gameover == true) {
        // Do nothing in our gameover state unless one of our players presses
        // a key to return to the main menu.
        for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
            if (inputs->menu.inputs[i] & MINPUT_OK) {
                return INGAME_RESULT_GAMEOVER;
            }
            if (inputs->menu.inputs[i] & MINPUT_CANCEL) {
                return INGAME_RESULT_GAMEOVER;
            }
        }

        return INGAME_RESULT_OK;
    } else if (ingame->countdown > 0) {
        // Play some countdown sounds
        if (ingame->countdown == MINO_FPS * 2) {
            // Ready
            audio_playsound(g_sound_ready);
        } else if (ingame->countdown == MINO_FPS) {
            // Go
            audio_playsound(g_sound_go);
        }

        // Do nothing except keep the count running.
        ingame->countdown -= 1;
        return INGAME_RESULT_OK;
    }

    // TODO: We need to leave the door open for pausing in netgames.  This
    //       requires running the game in the background of the pause menu.

    // Check to see if we need to pause the game
    for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
        if (inputs->interface.inputs[i] & IINPUT_PAUSE) {
            return INGAME_RESULT_PAUSE;
        }
    }

    if (environment_frame(ingame->environment, &inputs->game) == false) {
        // Start our Game Over state
        ingame->gameover = true;
        audio_playsound(g_sound_gameover);
    }

    return INGAME_RESULT_OK;
}

/**
 * Navigate to the proper destination
 */
static void ingame_navigate(screens_t* screens, int result) {
    screen_t* screen = screens_top(screens);

    switch ((playmenu_result_t)result) {
    case INGAME_RESULT_OK:
        // Do nothing
        break;
    case INGAME_RESULT_ERROR:
        screens_pop_until(screens, SCREEN_MAINMENU);
        break;
    case INGAME_RESULT_GAMEOVER:
        // TODO: Handle high scores here.
        screens_pop_until(screens, SCREEN_MAINMENU);
        break;
    case INGAME_RESULT_PAUSE:
        screens_push(screens, pausemenu_new(screen, 0));
        break;
    }
}

/**
 * Render the ingame screen
 */
static void ingame_render(screen_t* screen) {
    ingame_t* ingame = screen->screen.ingame;

    environment_draw(ingame->environment);

    if (ingame->gameover == true) {
        render()->draw_font(vec2i(100, 100), "GAME OVER");
    } else if (ingame->countdown > MINO_FPS) {
        render()->draw_font(vec2i(100, 100), "READY");
    } else if (ingame->countdown > 0) {
        render()->draw_font(vec2i(100, 100), " GO! ");
    }
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
    ingame->countdown = MINO_FPS * 2;
    ingame->gameover = false;

    screen.config = ingame_screen;
    screen.screen.ingame = ingame;
    return screen;
}

bool ingame_restart(screen_t* screen) {
    ingame_t* ingame = screen->screen.ingame;

    // Restart our environment
    if (environment_start(ingame->environment) == false) {
        return false;
    }

    ingame->countdown = MINO_FPS * 2;
    ingame->gameover = false;
}
