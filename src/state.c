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

    for (size_t i = 0;i < state->board_count;i++) {
        state->boards[i] = board_new();
        if (state->boards[i] == NULL) {
            state_delete(state);
            return NULL;
        }
    }

    // Initialize default pieces.
    pieces_init();

    state->pieces[0] = &g_j_piece;
    state->pieces[1] = &g_l_piece;
    state->pieces[2] = &g_s_piece;
    state->pieces[3] = &g_z_piece;
    state->pieces[4] = &g_t_piece;
    state->pieces[5] = &g_i_piece;
    state->pieces[6] = &g_o_piece;

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

    free(state);
}

/**
 * Given a particular gamestate, mutate it based on a particular event.
 * 
 * @param state The state to start from.
 * @param event The event to run on the gamestate.
 */
void state_frame(state_t* state, events_t events) {
    board_t* board = state->boards[0];

    if (board->piece == NULL) {
        board->piece = piece_new(&g_t_piece);
    }

    piece_t* piece = board->piece;

    // Handle movement.
    int dx = 0;
    if (events & EVENT_LEFT) {
        dx -= 1;
    }
    if (events & EVENT_RIGHT) {
        dx += 1;
    }
    if (dx != 0) {
        if (board_test_piece(board, piece->config, piece->x + dx, piece->y, piece->rot)) {
            piece->x += dx;
        }
    }

    // TODO: This is just some testing stuff, drops don't work like this.
    int dy = 0;
    if (events & EVENT_HARDDROP) {
        dy -= 1;
    }
    if (events & EVENT_SOFTDROP) {
        dy += 1;
    }
    if (dy != 0) {
        if (board_test_piece(board, piece->config, piece->x, piece->y + dy, piece->rot)) {
            piece->y += dy;
        }
    }

    // Handle rotation.
    int drot = 0;
    if (events & EVENT_CCW) {
        drot -= 1;
    }
    if (events & EVENT_CW) {
        drot += 1;
    }
    if (events & EVENT_180) {
        drot -= 2;
    }
    if (drot != 0) {
        int prot = (piece->rot + drot);
        if (prot < 0) {
            prot += piece->config->data_count;
        }
        prot %= piece->config->data_count;
        if (board_test_piece(board, piece->config, piece->x, piece->y, prot)) {
            piece->rot = prot;
        }
    }

    // Whatever happens, our gamtic always increases by one.
    state->tic += 1;
}
