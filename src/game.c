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

#include "event.h"
#include "game.h"
#include "state.h"

/**
 * What kind of screen we're looking at.
 */
typedef enum {
    SCREEN_MENU,
    SCREEN_INGAME,
    MAX_SCREENS
} gamescreen_t;

/**
 * The top-level "game" struct.
 */
typedef struct {
    gamescreen_t screen;
} game_t;

static game_t g_game = { SCREEN_INGAME };
static state_t* g_gamestate;

/**
 * Initialize the game.
 */
void game_init(void) {
    g_gamestate = state_new();
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    state_delete(g_gamestate);
    g_gamestate = NULL;
}

/**
 * Run a single fame of the game.
 */
void game_frame(gameinputs_t* inputs) {
    switch (g_game.screen) {
    case SCREEN_MENU:
        break;
    case SCREEN_INGAME:
        state_frame(g_gamestate, inputs->game);
        break;
    default:
        // Do nothing
        break;
    }
}
