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

#include "playmenu.h"

#include <stdlib.h>

#include "gametype.h"
#include "ingame.h"
#include "menu.h"
#include "ruleset.h"
#include "screen.h"

// Forward declarations
typedef struct ruleset_s ruleset_t;

typedef enum {
    PLAYMENU_RESULT_BACK = -1,
} playmenu_result_t;

typedef struct playmenu_s {
    /**
     * Current ruleset
     * 
     * This is not an owning reference.  Do not free it.
     */
    ruleset_t* ruleset;

    /**
     * List of gametypes that can be played.
     */
    menulist_t* list;

    /**
     * Currently selected gametype.
     */
    size_t selected;

    /**
     * Held keys.
     */
    gameholds_t holds;
} playmenu_t;

/**
 * Process inputs on the main menu
 */
static int playmenu_frame(screen_t* screen, const gameinputs_t* inputs) {
    playmenu_t* menu = screen->screen.playmenu;
    playerinputs_t minputs = input_menu_filter(&menu->holds, inputs);

    size_t gametype_count = menulist_count(menu->list);
    if (minputs.inputs[0] & MINPUT_UP) {
        menu->selected = (menu->selected + gametype_count - 1) % gametype_count;
    }
    if (minputs.inputs[0] & MINPUT_DOWN) {
        menu->selected = (menu->selected + 1) % gametype_count;
    }
    if (minputs.inputs[0] & MINPUT_OK) {
        return menu->selected + 1;
    }
    if (minputs.inputs[0] & MINPUT_CANCEL) {
        return PLAYMENU_RESULT_BACK;
    }

    return 0;
}

/**
 * Navigate to the proper destination
 */
static void playmenu_navigate(screens_t* screens, int result) {
    playmenu_t* menu = screens_top(screens)->screen.playmenu;

    if (result == PLAYMENU_RESULT_BACK) {
        screens_pop(screens);
        return;
    }

    // Which menu item did we select?
    const menuitem_t* item = menulist_get(menu->list, menu->selected);
    if (item == NULL) {
        return;
    }

    // Find our gametype.
    vfile_t* script = gametype_find_script(menu->ruleset->name, item->value);
    if (script == NULL) {
        return;
    }

    // To go ingame, the gametype must be initialized.
    /*gametype_t* gametype = gametype_new(menu->ruleset->lua, script, item->value);
    buffer_delete(script);
    if (gametype == NULL) {
        return;
    }

    // We are passing ownership of gametype to the ingame struct (for now).
    screen_t ingame = ingame_new(menu->ruleset, gametype);
    if (ingame.config.type == SCREEN_NONE) {
        return;
    }
    screens_push(screens, ingame);*/
}

/**
 * Render the main menu
 */
static void playmenu_render(screen_t* screen) {
    playmenu_t* menu = screen->screen.playmenu;

    render()->draw_mainmenu_bg();

    render()->draw_font(vec2i(100, 4), "Select Gamemode");

    // Draw our game modes
    size_t count = menulist_count(menu->list);
    for (size_t i = 0;i < count;i++) {
        size_t y = 16 + (i * 8);
        const menuitem_t* item = menulist_get(menu->list, i);
        if (item->label != NULL) {
            render()->draw_font(vec2i(50, y), item->label);
        } else {
            render()->draw_font(vec2i(50, y), item->value);
        }
        if (menu->selected == i) {
            render()->draw_font(vec2i(42, y), ">");
            if (item->help != NULL) {
                render()->draw_font(vec2i(4, 228), item->help);
            }
        }
    }
}

/**
 * Free main menu screen
 */
static void playmenu_delete(screen_t* screen) {
    if (screen->screen.playmenu != NULL) {
        if (screen->screen.playmenu->list != NULL) {
            menulist_delete(screen->screen.playmenu->list);
            screen->screen.playmenu->list = NULL;
        }

        free(screen->screen.playmenu);
        screen->screen.playmenu = NULL;
    }
}

screen_config_t playmenu_screen = {
    SCREEN_PLAYMENU,
    playmenu_frame,
    playmenu_navigate,
    playmenu_render,
    playmenu_delete
};

/**
 * Allocate play menu screen
 */
screen_t playmenu_new(ruleset_t* ruleset) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    playmenu_t* menu = calloc(1, sizeof(playmenu_t));
    if (menu == NULL) {
        return screen;
    }
    input_holds_init(&menu->holds);

    // Get a list of gametypes for this ruleset.
    menulist_t* menulist = ruleset_get_gametypes(ruleset);
    if (menulist == NULL) {
        free(menu);
        return screen;
    }

    screen.config = playmenu_screen;
    screen.screen.playmenu = menu;
    screen.screen.playmenu->list = menulist;
    screen.screen.playmenu->ruleset = ruleset;
    return screen;
}
