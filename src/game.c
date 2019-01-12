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

#include "audio.h"
#include "game.h"
#include "render.h"
#include "state.h"
#include "vfs.h"

/**
 * What kind of screen we're looking at.
 */
typedef enum {
    SCREEN_MENU,
    SCREEN_INGAME_PLAY,
    SCREEN_INGAME_GAMEOVER,
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
    vfs_init();
    g_game.screen = SCREEN_INGAME_PLAY;
    g_game.state = state_new();
    g_game.render = render_init();
    audio_init();
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    audio_deinit();

    if (g_game.render != NULL) {
        render_deinit(g_game.render);
        g_game.render = NULL;
    }

    if (g_game.state != NULL) {
        state_delete(g_game.state);
        g_game.state = NULL;
    }

    vfs_deinit();
}

/**
 * Run a single fame of the game.
 */
void game_frame(const gameevents_t* events) {
    switch (g_game.screen) {
    case SCREEN_MENU:
        break;
    case SCREEN_INGAME_PLAY:
        if (state_frame(g_game.state, &(events->game)) == STATE_RESULT_GAMEOVER) {
            audio_playsound(g_sound_gameover);
            g_game.screen = SCREEN_INGAME_GAMEOVER;
        }
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
    case SCREEN_INGAME_GAMEOVER:
    case SCREEN_INGAME_PLAY:
        return g_game.render->draw_state(g_game.state);
        break;
    default:
        break;
    }

    // Unreachable
    return NULL;
}
