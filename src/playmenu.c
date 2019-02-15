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

#include "menu.h"
#include "ruleset.h"
#include "screen.h"

// Forward declarations
typedef struct ruleset_s ruleset_t;

typedef enum {
    PLAYMENU_RESULT_BACK = -1,
} gametype_result_t;

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
 * Process events on the main menu
 */
static int playmenu_frame(screen_t* screen, const gameevents_t* events) {
    playmenu_t* menu = screen->screen.playmenu;
    playerevents_t mevents = event_menu_filter(&menu->holds, events);

    size_t gametype_count = menulist_count(menu->list);
    if (mevents.events[0] & MEVENT_UP) {
        menu->selected = (menu->selected + gametype_count - 1) % gametype_count;
    }
    if (mevents.events[0] & MEVENT_DOWN) {
        menu->selected = (menu->selected + 1) % gametype_count;
    }
    if (mevents.events[0] & MEVENT_OK) {
        return menu->selected + 1;
    }
    if (mevents.events[0] & MEVENT_CANCEL) {
        return PLAYMENU_RESULT_BACK;
    }

    return 0;
}

/**
 * Navigate to the proper destination
 */
static void playmenu_navigate(screens_t* screens, int result) {
    if (result == PLAYMENU_RESULT_BACK) {
        screens_pop(screens);
    }
}

/**
 * Render the main menu
 */
static void playmenu_render(screen_t* screen, render_module_t* render) {
    playmenu_t* menu = screen->screen.playmenu;

    render->draw_mainmenu_bg();

    render->draw_font(vec2i(100, 4), "Select Gamemode");

    // Draw our game modes
    size_t count = menulist_count(menu->list);
    for (size_t i = 0;i < count;i++) {
        size_t y = 16 + (i * 8);
        const menuitem_t* item = menulist_get(menu->list, i);
        if (item->label != NULL) {
            render->draw_font(vec2i(50, y), item->label);
        } else {
            render->draw_font(vec2i(50, y), item->value);
        }
        if (menu->selected == i) {
            render->draw_font(vec2i(42, y), ">");
            if (item->help != NULL) {
                render->draw_font(vec2i(4, 228), item->help);
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
