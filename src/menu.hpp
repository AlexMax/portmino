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

#pragma once

#include "define.hpp"

/**
 * A simple menu item
 */
typedef struct {
    /**
     * Hidden "value" of the menu item.
     */
    char* value;

    /**
     * Visible label of the menu item.
     */
    char* label;

    /**
     * Visible help text of the menu item.
     */
    char* help;

    /**
     * Position.
     */
    int position;
} menuitem_t;

typedef struct menulist_s menulist_t;

/**
 * Parameters for menulist_push
 */
typedef struct {
    const char* value;
    const char* label;
    const char* help;
    int position;
} menulist_params_t;

menulist_t* menulist_new(void);
void menulist_delete(menulist_t* menulist);
bool menulist_push(menulist_t* menulist, const menulist_params_t* params);
size_t menulist_count(menulist_t* menulist);
const menuitem_t* menulist_get(menulist_t* menulist, size_t index);
