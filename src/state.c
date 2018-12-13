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
        abort();
    }

    state->background = NULL;
    state->field = field_new();
    state->tic = 0;

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
    }

    if (state->field != NULL) {
        field_delete(state->field);
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
    // Whatever happens, our gamtic always increases by one.
    state->tic += 1;
}

#include <stdio.h>

/**
 * Dump the contents of state to standard out.
 * 
 * @param state The state to dump.
 */
void state_debug(state_t* state) {
    field_config_t config = state->field->config;

    printf("tic: %d\n", state->tic);
    printf("width: %d\n", config.width);
    printf("height: %d\n",config.height);
    printf("visible_height: %d\n", config.visible_height);

    size_t invis = (config.height - config.visible_height) * config.width;
    for (size_t i = 0;i < state->field->data.size;i++) {
        field_cell_t c = state->field->data.data[i];
        switch (c) {
        case 1:
            fputs("#", stderr);
            break;
        default:
            if (invis > i) {
                fputs(",", stderr);
            } else {
                fputs(".", stderr);
            }
        }
        if (i % config.width == config.width - 1) {
            fputs("\n", stderr);
        }
    }
}
