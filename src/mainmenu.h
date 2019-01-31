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

#include "define.h"

#include "event.h"

typedef enum {
    MAINMENU_RESULT_NONE,
    MAINMENU_RESULT_PLAY,
    MAINMENU_RESULT_RECORDS,
    MAINMENU_RESULT_RULESET,
    MAINMENU_RESULT_OPTIONS,
    MAINMENU_RESULT_QUIT,
} mainmenu_result_t;

typedef struct {
    /**
     * Currently selected option.
     */
    uint8_t selected;
} mainmenu_t;

mainmenu_t* mainmenu_new(void);
void mainmenu_delete(mainmenu_t* menu);
mainmenu_result_t mainmenu_frame(mainmenu_t* menu, const playerevents_t* events);
void mainmenu_render(mainmenu_t* menu);
