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

/**
 * Allocate main menu state
 */
mainmenu_t* mainmenu_new(void) {
    mainmenu_t* menu = calloc(1, sizeof(mainmenu_t));
    if (menu == NULL) {
        return NULL;
    }

    return menu;
}

/**
 * Free main menu state
 */
void mainmenu_delete(mainmenu_t* menu) {
    free(menu);
}

/**
 * Process events on the main menu.
 */
mainmenu_result_t mainmenu_frame(mainmenu_t* menu, const playerevents_t* events) {
    if (events->events[0] & MEVENT_UP) {
        menu->selected = (menu->selected + 4) % 5;
    }
    if (events->events[0] & MEVENT_DOWN) {
        menu->selected = (menu->selected + 1) % 5;
    }
    if (events->events[0] & MEVENT_OK) {
        return menu->selected + 1;
    }

    return 0;
}

/**
 * Render the main menu in a platform-agnostic way.
 */
void mainmenu_render(mainmenu_t* menu) {
    (void)menu;
}
