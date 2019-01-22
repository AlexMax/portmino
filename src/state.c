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

#include <stdlib.h>

#include "ruleset.h"
#include "state.h"

/**
 * Create our gamestate.
 *
 * @return state_t* A newly created gamestate.
 */
state_t* state_new(void) {
    state_t* state = calloc(1, sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->background = NULL;
    state->board_count = 1;
    state->player_count = 1;
    state->next_count = 1;
    state->tic = 0;

    state->ruleset = ruleset_new();
    if (state->ruleset == NULL) {
        state_delete(state);
        return NULL;
    }

    for (size_t i = 0;i < state->board_count;i++) {
        state->boards[i] = board_new(state->ruleset, i);
        if (state->boards[i] == NULL) {
            state_delete(state);
            return NULL;
        }
    }

    state->nexts = calloc(state->next_count, sizeof(next_t));
    if (state->nexts == NULL) {
        state_delete(state);
        return NULL;
    }

    for (size_t i = 0;i < state->next_count;i++) {
        state->nexts[i] = next_new(state->ruleset, i);
        if (state->nexts[i] == NULL) {
            state_delete(state);
            return NULL;
        }
    }

    return state;
}

/**
 * Delete our gamestate.
 * 
 * @param state The gamestate to delete.
 */
void state_delete(state_t* state) {
    if (state->background != NULL) {
        free(state->background);
        state->background = NULL;
    }

    for (size_t i = 0;i < state->board_count;i++) {
        if (state->boards[i] != NULL) {
            board_delete(state->boards[i]);
            state->boards[i] = NULL;
        }
    }

    if (state->nexts != NULL) {
        for (size_t i = 0;i < state->next_count;i++) {
            if (state->nexts[i] != NULL) {
                next_delete(state->nexts[i]);
                state->nexts[i] = NULL;
            }
        }

        free(state->nexts);
        state->nexts = NULL;
    }

    if (state->ruleset != NULL) {
        ruleset_delete(state->ruleset);
        state->ruleset = NULL;
    }

    free(state);
}

/**
 * Given a particular gamestate, mutate it based on a particular event.
 * 
 * @param state The state to start from.
 * @param playerevents The events to run on the gamestate.
 */
state_result_t state_frame(state_t* state, const playerevents_t* playerevents) {
    // Whatever happens, our gametic increases by one.  Tic 0 does not exist.
    state->tic += 1;

    if (state->tic == 0) {
        // How have you been playing for this long?
        return STATE_RESULT_ERROR;
    }

    ruleset_result_t result = ruleset_frame(state->ruleset, state, playerevents);
    switch (result) {
        case RULESET_RESULT_OK: return STATE_RESULT_OK;
        case RULESET_RESULT_ERROR: return STATE_RESULT_ERROR;
        case RULESET_RESULT_TOPOUT: return STATE_RESULT_GAMEOVER;
    }

    // unreachable
    return STATE_RESULT_ERROR;
}
