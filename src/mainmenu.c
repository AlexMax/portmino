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

#include "mainmenu.h"

#include <stdlib.h>

#include "event.h"
#include "render.h"

typedef enum {
    MAINMENU_RESULT_PLAY = 1,
    MAINMENU_RESULT_RECORDS,
    MAINMENU_RESULT_RULESET,
    MAINMENU_RESULT_OPTIONS,
    MAINMENU_RESULT_QUIT,
} mainmenu_result_t;

/**
 * Process events on the main menu
 */
static int mainmenu_frame(screen_t* screen, const gameevents_t* events) {
    mainmenu_t* menu = screen->screen.menu;
    playerevents_t mevents = event_menu_filter(&menu->holds, events);

    if (mevents.events[0] & MEVENT_UP) {
        menu->selected = (menu->selected + 4) % 5;
    }
    if (mevents.events[0] & MEVENT_DOWN) {
        menu->selected = (menu->selected + 1) % 5;
    }
    if (mevents.events[0] & MEVENT_OK) {
        return menu->selected + 1;
    }

    return 0;
}

/**
 * Navigate to the proper destination
 */
static void mainmenu_navigate(screens_t* screens, int result) {
    switch ((mainmenu_result_t)result) {
    case MAINMENU_RESULT_PLAY:
        screens_push(screens, mainmenu_new());
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
}

/**
 * Render the main menu
 */
static void mainmenu_render(screen_t* screen, render_module_t* render) {
    mainmenu_t* menu = screen->screen.menu;

    render->clear();

    render->draw_font(vec2i(100, 50), "Portmino v0.1");

    render->draw_font(vec2i(100, 100), "Play");
    render->draw_font(vec2i(100, 110), "Records");
    render->draw_font(vec2i(100, 120), "Ruleset");
    render->draw_font(vec2i(100, 130), "Options");
    render->draw_font(vec2i(100, 140), "Quit");

    int y = 100 + (10 * menu->selected);
    render->draw_font(vec2i(92, y), ">");
}

/**
 * Free main menu screen
 */
static void mainmenu_delete(screen_t* screen) {
    if (screen->screen.menu != NULL) {
        free(screen->screen.menu);
        screen->screen.menu = NULL;
    }
}

screen_config_t mainmenu_screen = {
    SCREEN_MAINMENU,
    mainmenu_frame,
    mainmenu_navigate,
    mainmenu_render,
    mainmenu_delete
};

/**
 * Allocate main menu screen
 */
screen_t mainmenu_new(void) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    mainmenu_t* menu = calloc(1, sizeof(mainmenu_t));
    if (menu == NULL) {
        return screen;
    }
    menu->selected = 0;
    event_holds_init(&menu->holds);

    screen.config = mainmenu_screen;
    screen.screen.menu = menu;
    return screen;
}
