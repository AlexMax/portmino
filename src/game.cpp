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

#include "game.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.hpp"

#include "audio.hpp"
#include "error.hpp"
#include "frontend.hpp"
#include "mainmenu.hpp"
#include "render.hpp"
#include "ruleset.hpp"
#include "screen.hpp"
#include "script.hpp"
#include "vfs.hpp"

/**
 * The global screen stack.
 */
static screens_t g_screens;

/**
 * Our global Lua interpreter state that we use for everything.
 */
static lua_State* g_lua;

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

    if (render_init(RENDERER_SOFTWARE) == false) {
        return false;
    }

    if (!audio_init()) {
        return false;
    }

    if ((g_lua = script_newstate()) == NULL) {
        return false;
    }

    // We start at the main menu.
    screen_t mainmenu = mainmenu_new(g_lua);
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
    if (g_lua != NULL) {
        lua_close(g_lua);
        g_lua = NULL;
    }

    audio_deinit();
    render_deinit();
    vfs_deinit();
}

/**
 * Run a single fame of the game
 */
void game_frame(const gameinputs_t* inputs) {
    // Pass our inputs to the screen that needs it.
    screens_frame(&g_screens, inputs);

    // Display all non-fatal errors.
    char* err;
    while ((err = error_pop()) != NULL) {
        fprintf(stderr, "frame error: %s\n", err);
    }
}

/**
 * Draw the frame, returning the render context so we can blit it.
 */
void* game_draw(void) {
    // Render the proper screen.
    screens_render(&g_screens);

    // Display all non-fatal errors.
    char* err;
    while ((err = error_pop()) != NULL) {
        fprintf(stderr, "draw error: %s\n", err);
    }

    // Return the context.
    return render()->context();
}
