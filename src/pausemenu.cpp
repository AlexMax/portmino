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

#include "pausemenu.hpp"

#include <stdlib.h>

#include "audio.hpp"
#include "ingame.hpp"
#include "screen.hpp"

typedef enum {
    PAUSEMENU_RESULT_RESUME = 1,
    PAUSEMENU_RESULT_RESTART,
    PAUSEMENU_RESULT_QUIT,
} pausemenu_result_t;

typedef struct pausemenu_s {
    /**
     * Background screen
     *
     * This is not an owning pointer.  Do not free it.
     */
    screen_t* background;

    /**
     * Currently selected option.
     */
    uint8_t selected;

    /**
     * Held keys.
     */
    gameholds_t holds;
} pausemenu_t;

/**
 * Process inputs on the pause menu
 */
static int pausemenu_frame(screen_t* screen, const gameinputs_t* inputs) {
    pausemenu_t* menu = screen->screen.pausemenu;
    playerinputs_t minputs = input_menu_filter(&menu->holds, inputs);

    if (minputs.inputs[0] & MINPUT_UP) {
        menu->selected = (menu->selected + 2) % 3;
        audio_playsound("cursor");
    }
    if (minputs.inputs[0] & MINPUT_DOWN) {
        menu->selected = (menu->selected + 1) % 3;
        audio_playsound("cursor");
    }
    if (minputs.inputs[0] & MINPUT_OK) {
        return menu->selected + 1;
    }
    if (minputs.inputs[0] & MINPUT_CANCEL) {
        return PAUSEMENU_RESULT_RESUME;
    }

    return 0;
}

/**
 * Navigate to the proper destination
 */
static void pausemenu_navigate(screens_t* screens, int result) {
    switch (result) {
    case PAUSEMENU_RESULT_RESUME:
        screens_pop(screens);
        audio_playsound("ok");
        break;
    case PAUSEMENU_RESULT_RESTART:
        screens_pop(screens);
        ingame_restart(screens_top(screens));
        break;
    case PAUSEMENU_RESULT_QUIT:
        screens_pop_until(screens, SCREEN_MAINMENU);
        audio_playsound("ok");
        break;
    }
}

/**
 * Render the pause menu
 */
static void pausemenu_render(screen_t* screen) {
    pausemenu_t* menu = screen->screen.pausemenu;

    // Render the game in the background first
    menu->background->config.render(menu->background);

    // Draw the menu background
    render()->draw_box(vec2i(88, 96), vec2i(80, 36));

    // Now render our actual pause menu
    render()->draw_font(vec2i(100, 100), "Continue");
    render()->draw_font(vec2i(100, 110), "Restart");
    render()->draw_font(vec2i(100, 120), "Quit");

    int y = 100 + (10 * menu->selected);
    render()->draw_font(vec2i(92, y), ">");
}

/**
 * Free pause menu screen
 */
static void pausemenu_delete(screen_t* screen) {
    if (screen->screen.pausemenu != NULL) {
        free(screen->screen.pausemenu);
        screen->screen.pausemenu = NULL;
    }
}

screen_config_t pausemenu_screen = {
    SCREEN_PAUSEMENU,
    pausemenu_frame,
    pausemenu_navigate,
    pausemenu_render,
    pausemenu_delete
};

/**
 * Allocate pause menu screen
 */
screen_t pausemenu_new(screen_t* background, uint8_t player) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    pausemenu_t* menu = calloc(1, sizeof(pausemenu_t));
    if (menu == NULL) {
        return screen;
    }

    input_holds_init(&menu->holds);

    screen.config = pausemenu_screen;
    screen.screen.pausemenu = menu;
    screen.screen.pausemenu->background = background;

    return screen;
}
