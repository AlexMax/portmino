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
#include "ruleset.h"
#include "screen.h"
#include "vfs.h"

/**
 * The global screen stack.
 */
static screens_t g_screens;

/**
 * The current in-use renderer.
 */
static render_module_t* g_render;

/**
 * Initialize the game
 * 
 * Pass in argc and argv so we can parse options and initialize the VFS.
 */
bool game_init(int argc, char** argv) {
    // Initialize subsystems.
    bool ok;
    if (argc > 0) {
        // Started from command line
        ok = vfs_init(argv[0]);
    } else {
        // Shared library or something
        ok = vfs_init(NULL);
    }
    if (!ok) {
        return false;
    }

    if ((g_render = render_init()) == NULL) {
        return false;
    }

    if (!audio_init()) {
        return false;
    }

    // We start at the main menu.
    screen_t mainmenu = mainmenu_new();
    if (mainmenu.config.type == SCREEN_NONE) {
        return false;
    }

    screens_init(&g_screens);
    screens_push(&g_screens, mainmenu);

    return true;
}

/**
 * Clean up the game.
 */
void game_deinit(void) {
    // Destroy the screen stack.
    screens_deinit(&g_screens);

    // Deinit subsystems.
    audio_deinit();
    if (g_render != NULL) {
        render_deinit(g_render);
        g_render = NULL;
    }
    vfs_deinit();
}

/**
 * Run a single fame of the game
 */
void game_frame(const gameevents_t* events) {
    // Pass our events to the screen that needs it.
    screens_frame(&g_screens, events);
}

/**
 * Draw the frame, returning the render context so we can blit it.
 */
void* game_draw(void) {
    // Render the proper screen.
    screens_render(&g_screens, g_render);

    // Return the context.
    return g_render->context();
}
