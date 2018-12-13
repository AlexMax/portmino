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

#include "game.h"
#include "render.h"
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
    state_t* state;
    render_module_t* render;
} game_t;

static game_t g_game;

/**
 * Initialize the game.
 */
void game_init(void) {
    g_game.screen = SCREEN_INGAME;
    g_game.state = state_new();
    g_game.render = render_init();
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    if (g_game.state != NULL) {
        state_delete(g_game.state);
        g_game.state = NULL;
    }

    if (g_game.render != NULL) {
        render_deinit(g_game.render);
        g_game.render = NULL;
    }
}

/**
 * Run a single fame of the game.
 */
void game_frame(gameinputs_t* inputs) {
    switch (g_game.screen) {
    case SCREEN_MENU:
        break;
    case SCREEN_INGAME:
        state_frame(g_game.state, inputs->game);
        break;
    default:
        break;
    }
}

/**
 * Draw the frame, returning the render context so we can blit it.
 */
void* game_draw(void) {
    switch (g_game.screen) {
    case SCREEN_MENU:
        break;
    case SCREEN_INGAME:
        return g_game.render->draw_state(g_game.state);
        break;
    default:
        break;
    }

    // Unreachable
    return NULL;
}
