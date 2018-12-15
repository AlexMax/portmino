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

#include "state.h"

/**
 * Create our gamestate.
 *
 * @return state_t* A newly created gamestate.
 */
state_t* state_new(void) {
    state_t* state = malloc(sizeof(state_t));
    if (state == NULL) {
        return NULL;
    }

    state->background = NULL;
    state->board_count = 1;
    state->tic = 0;

    state->boards = malloc(sizeof(board_t*) * state->board_count);
    for (size_t i = 0;i < state->board_count;i++) {
        state->boards[i] = board_new();
        if (state->boards[i] == NULL) {
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

    if (state->boards != NULL) {
        for (size_t i = 0;i < state->board_count;i++) {
            if (state->boards[i] != NULL) {
                board_delete(state->boards[i]);
                state->boards = NULL;
            }
        }

        free(state->boards);
        state->boards = NULL;
    }

    free(state);
}

/**
 * Given a particular gamestate, mutate it based on a particular event.
 * 
 * @param state The state to start from.
 * @param event The event to run on the gamestate.
 */
void state_frame(state_t* state, events_t events) {
    // board_t* field = state->fields[0];
    // for (int i = 0;i < field->data.size;i++) {
    //     field->data.data[i] = i % 8;
    // }

    // Whatever happens, our gamtic always increases by one.
    state->tic += 1;
}
