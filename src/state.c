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

#include "state.h"

#include <stdlib.h>

#include "board.h"
#include "ruleset.h"

/**
 * Create our gamestate.
 *
 * @return state_t* A newly created gamestate.
 */
state_t* state_new(ruleset_t* ruleset) {
    state_t* state = calloc(1, sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->background = NULL;
    state->board_count = 1;
    state->player_count = 1;
    state->next_count = 1;
    state->tic = 0;

    for (size_t i = 0;i < state->board_count;i++) {
        state->boards[i] = board_new(ruleset, i);
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
        state->nexts[i] = next_new(ruleset, i);
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
    if (state == NULL) {
        return;
    }

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

    free(state);
}

/**
 * Given a particular gamestate, mutate it based on a particular event.
 * 
 * @param state The state to start from.
 * @param playerevents The events to run on the gamestate.
 */
bool state_frame(state_t* state) {
    // Whatever happens, our gametic increases by one.  Tic 0 does not exist.
    state->tic += 1;

    if (state->tic == 0) {
        // How have you been playing for this long?
        return false;
    }

    return true;
}
