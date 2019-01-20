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
#include "board.h"
#include "event.h"
#include "state.h"

// Right now we only support two boards.
#define MAX_BOARDS 2

typedef struct ruleset_s ruleset_t;
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
     * In-use player count.
     */
    size_t player_count;

    /**
     * In-use ruleset.
     */
    ruleset_t* ruleset;
} state_t;

/**
 * The result of advancing our state a single frame.
 */
typedef enum {
    /**
     * Nothing surprising happened in this frame.
     */
    STATE_RESULT_OK,

    /**
     * State reached an indeterminate or error result.
     */
    STATE_RESULT_ERROR,

    /**
     * State reached a "game over" result, usually a topout.
     */
    STATE_RESULT_GAMEOVER,

    /**
     * State reached a "winning" result, like clearing all the lines.
     */
    STATE_RESULT_SUCCESS,
} state_result_t;

state_t* state_new(void);
void state_delete(state_t* state);
state_result_t state_frame(state_t* state, const playerevents_t* playerevents);
void state_debug(state_t* state);
