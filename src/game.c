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
#include "mainmenu.h"
#include "render.h"
#include "screen.h"
#include "state.h"
#include "vfs.h"

/**
 * The top-level "game" struct.
 */
typedef struct {
    screens_t screens;
    gameholds_t holds;
    render_module_t* render;
} game_t;

static game_t g_game;

/**
 * Initialize the game.
 */
void game_init(void) {
    // Initialize subsystems.
    vfs_init();
    g_game.render = render_init();
    audio_init();

    // Reset holds.
    event_holds_reset(&g_game.holds);

    // We start at the main menu using the default ruleset.
    screens_init(&g_game.screens);
    screens_push(&g_game.screens, mainmenu_new());
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    screens_deinit(&g_game.screens);
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
    // Pass our events to the screen that needs it.
    screens_frame(&g_game.screens, events);
}

/**
 * Draw the frame, returning the render context so we can blit it.
 */
void* game_draw(void) {
    // Render the proper screen.
    screens_render(&g_game.screens, g_game.render);

    // Return the context.
    return g_game.render->context();
}
