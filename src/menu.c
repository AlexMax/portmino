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

#include "menu.h"

#include <stdlib.h>
#include <string.h>

/**
 * A list of menu items, intended for a menu
 */
typedef struct menulist_s {
    /**
     * List of menu items.
     */
    menuitem_t** items;

    /**
     * Number of items in the menu list.
     */
    size_t count;
} menulist_t;

/**
 * Create a new menulist.
 */
menulist_t* menulist_new(void) {
    return calloc(1, sizeof(menulist_t));
}

/**
 * Delete a menulist.
 */
void menulist_delete(menulist_t* menulist) {
    if (menulist->items != NULL) {
        for (size_t i = 0;i < menulist->count;i++) {
            if (menulist->items[i] != NULL) {
                if (menulist->items[i]->value != NULL) {
                    free(menulist->items[i]->value);
                    menulist->items[i]->value = NULL;
                }
                if (menulist->items[i]->label != NULL) {
                    free(menulist->items[i]->label);
                    menulist->items[i]->label = NULL;
                }
                if (menulist->items[i]->help != NULL) {
                    free(menulist->items[i]->help);
                    menulist->items[i]->help = NULL;
                }
                free(menulist->items[i]);
            }
        }
        free(menulist->items);
        menulist->items = NULL;
    }
    free(menulist);
}

/**
 * Push a new item into the menulist.
 */
bool menulist_push(menulist_t* menulist, const menulist_params_t* params) {
    // Allocate the menu item
    menuitem_t* item = calloc(1, sizeof(menuitem_t));
    if (item == NULL) {
        return false;
    }

    item->value = strdup(params->value);
    if (item->value == NULL) {
        free(item);
        return false;
    }

    item->label = strdup(params->label);
    if (item->label == NULL) {
        free(item->value);
        free(item);
        return false;
    }

    item->help = strdup(params->help);
    if (item->help == NULL) {
        free(item->value);
        free(item->label);
        free(item);
        return false;
    }

    // The item is allocated, realloc the menulist.
    menuitem_t** newitems = realloc(menulist->items, sizeof(menuitem_t*) * (menulist->count + 1));
    if (newitems == NULL) {
        return false;
    }

    menulist->items = newitems;
    menulist->items[menulist->count] = item; // abuse of old count
    menulist->count += 1;

    return true;
}

/**
 * Return the size of the menulist
 */
size_t menulist_count(menulist_t* menulist) {
    return menulist->count;
}

/**
 * Get the menu item at the given index.
 */
const menuitem_t* menulist_get(menulist_t* menulist, size_t index) {
    if (index >= menulist->count) {
        return NULL;
    }

    return menulist->items[index];
}
