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

#include <stdint.h>

#include "board.h"
#include "event.h"
#include "state.h"

// Right now we only support two players.
#define MAX_BOARDS 2

typedef struct {
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
} state_t;

state_t* state_new(void);
void state_delete(state_t* state);
void state_frame(state_t* state, events_t events);
void state_debug(state_t* state);
