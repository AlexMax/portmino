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

#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "frontend.h"
#include "mainmenu.h"
#include "render.h"
#include "state.h"
#include "vfs.h"

/**
 * What kind of screen we're looking at.
 */
typedef enum {
    SCREEN_NONE,
    SCREEN_MAINMENU,
    SCREEN_INGAME_PLAY,
    SCREEN_INGAME_GAMEOVER,
} gamescreen_t;

/**
 * The top-level "game" struct.
 */
typedef struct {
    gamescreen_t screen_type;
    union screen_u {
        struct mainmenu_s {
            mainmenu_t* menu;
        } mainmenu;
        struct ingame_s {
            state_t* state;
        } ingame;
    } screen;
    gameholds_t holds;
    render_module_t* render;
} game_t;

static game_t g_game;

/**
 * Free anything that the game screen has allocated.
 */
static void game_screen_deinit(void) {
    // Free resources.
    switch (g_game.screen_type) {
    case SCREEN_NONE:
        break;
    case SCREEN_MAINMENU:
        if (g_game.screen.mainmenu.menu != NULL) {
            mainmenu_delete(g_game.screen.mainmenu.menu);
        }
        break;
    case SCREEN_INGAME_PLAY:
        if (g_game.screen.ingame.state != NULL) {
            state_delete(g_game.screen.ingame.state);
        }
        break;
    case SCREEN_INGAME_GAMEOVER:
        break;
    }

    // Set struct to null.
    memset(&g_game.screen, 0x00, sizeof(g_game.screen));
}

/**
 * Switch to the main menu.
 */
static void game_screen_mainmenu(void) {
    game_screen_deinit();

    g_game.screen_type = SCREEN_MAINMENU;
    g_game.screen.mainmenu.menu = mainmenu_new();
    if (g_game.screen.mainmenu.menu == NULL) {
        frontend_fatalerror("Main Menu allocation error");
        return;
    }
}

/**
 * Switch to ingame.
 */
static void game_screen_ingame(void) {
    game_screen_deinit();

    g_game.screen_type = SCREEN_INGAME_PLAY;
    g_game.screen.ingame.state = state_new();
    if (g_game.screen.ingame.state == NULL) {
        frontend_fatalerror("Ingame allocation error");
        return;
    }
}

/**
 * Initialize the game.
 */
void game_init(void) {
    vfs_init();
    g_game.render = render_init();
    audio_init();

    // Reset holds.
    event_holds_reset(&g_game.holds);

    // We start at the main menu using the default ruleset.
    game_screen_mainmenu();
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    game_screen_deinit();
    audio_deinit();

    if (g_game.render != NULL) {
        render_deinit(g_game.render);
        g_game.render = NULL;
    }

    vfs_deinit();
}

/**
 * Run a single fame of the game
 */
void game_frame(const gameevents_t* events) {
    playerevents_t ievents = event_interface_filter(&g_game.holds, events);
    playerevents_t mevents = event_menu_filter(&g_game.holds, events);

    switch (g_game.screen_type) {
    case SCREEN_NONE:
        break;
    case SCREEN_MAINMENU:
        {
            mainmenu_result_t res = mainmenu_frame(g_game.screen.mainmenu.menu, &mevents);
            switch (res) {
            case MAINMENU_RESULT_NONE:
                break;
            case MAINMENU_RESULT_PLAY:
                game_screen_ingame();
                break;
            case MAINMENU_RESULT_RECORDS:
                break;
            case MAINMENU_RESULT_RULESET:
                break;
            case MAINMENU_RESULT_OPTIONS:
                break;
            case MAINMENU_RESULT_QUIT:
                exit(EXIT_SUCCESS);
                break;
            }
        } break;
    case SCREEN_INGAME_PLAY:
        {
            state_result_t res = state_frame(g_game.screen.ingame.state, &(events->game));
            switch (res) {
            case STATE_RESULT_OK:
                break;
            case STATE_RESULT_SUCCESS:
            case STATE_RESULT_GAMEOVER:
                audio_playsound(g_sound_gameover);
                game_screen_mainmenu();
                break;
            case STATE_RESULT_ERROR:
                break;
            }
        } break;
    case SCREEN_INGAME_GAMEOVER:
        break;
    }
}

/**
 * Draw the frame, returning the render context so we can blit it.
 */
void* game_draw(void) {
    switch (g_game.screen_type) {
    case SCREEN_MAINMENU:
        return g_game.render->draw_mainmenu(g_game.screen.mainmenu.menu);
        break;
    case SCREEN_INGAME_GAMEOVER:
    case SCREEN_INGAME_PLAY:
        return g_game.render->draw_state(g_game.screen.ingame.state);
        break;
    default:
        break;
    }

    // Unreachable
    return NULL;
}
