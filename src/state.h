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

/**
 * Contains state for a particular player.
 */
typedef struct {
    /**
     * Tic that EVENT_LEFT began on.  Set to 0 if released.
     */
    uint32_t left_tic;

    /**
     * Tic that EVENT_RIGHT began on.  Set to 0 if released.
     */
    uint32_t right_tic;

    /**
     * Tic that lock delay started on.  Set to 0 if lock delay isn't in effect.
     */
    uint32_t lock_tic;

    /**
     * Tic that EVENT_HARDDROP began on.  Set to 0 if released.
     */
    uint32_t harddrop_tic;

    /**
     * Have we processed an EVENT_CCW last tic?
     */
    bool ccw_already;

    /**
     * Have we processed an EVENT_CW last tic?
     */
    bool cw_already;
} playstate_t;

typedef struct ruleset_s ruleset_t;
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

    /**
     * Players.
     */
    playstate_t playstates[MINO_MAX_PLAYERS];

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

void playstate_reset(playstate_t* ps);
state_t* state_new(void);
void state_delete(state_t* state);
state_result_t state_frame(state_t* state, const playerevents_t* playerevents);
void state_debug(state_t* state);
