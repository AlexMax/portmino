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

// Forward declarations.
typedef struct board_s board_t;
typedef struct next_s next_t;
typedef struct playerevents_s playerevents_t;
typedef struct ruleset_s ruleset_t;

// Right now we only support two boards.
#define MAX_BOARDS 2

typedef struct state_s {
    /**
     * Current gametic.
     */
    uint32_t tic;

    /**
     * Current background picture.
     */
    char* background;

    /**
     * Boards.
     */
    board_t* boards[MAX_BOARDS];

    /**
     * In-use board count.
     */
    size_t board_count;

    /**
     * Next piece buffers.
     */
    next_t** nexts;

    /**
     * Next piece buffer count.
     */
    size_t next_count;

    /**
     * In-use player count.
     */
    size_t player_count;
} state_t;

state_t* state_new(ruleset_t* ruleset);
void state_delete(state_t* state);
bool state_frame(state_t* state);
