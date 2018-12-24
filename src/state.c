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

#include "audio.h"
#include "state.h"

#define DEFAULT_DAS 12
#define DEFAULT_DAS_PERIOD 2

/**
 * Reset a player state struct to nothing.
 */
void playstate_reset(playstate_t* ps) {
    ps->left_tic = 0;
    ps->right_tic = 0;
    ps->ccw_already = 0;
    ps->cw_already = 0;
}

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
    state->player_count = 1;
    state->tic = 0;

    for (size_t i = 0;i < state->board_count;i++) {
        state->boards[i] = board_new();
        if (state->boards[i] == NULL) {
            state_delete(state);
            return NULL;
        }
    }

    for (size_t i = 0;i < state->player_count;i++) {
        playstate_reset(&(state->playstates[i]));
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

    free(state);
}

/**
 * Given a particular gamestate, mutate it based on a particular event.
 * 
 * @param state The state to start from.
 * @param event The event to run on the gamestate.
 */
state_result_t state_frame(state_t* state, events_t events) {
    // Whatever happens, our gametic increases by one.  Tic 0 does not exist.
    state->tic += 1;

    if (state->tic == 0) {
        // How have you been playing for this long?
        return STATE_RESULT_ERROR;
    }

    board_t* board = state->boards[0];

    // Get the next piece if we don't have one at this point.
    if (board->piece == NULL) {
        if (!board_next_piece(board)) {
            return STATE_RESULT_GAMEOVER;
        }
        audio_playsound(g_sound_piece0);
    }

    piece_t* piece = board->piece;

    // Handle gravity.
    if (state->tic % 60 == 0) {
        if (board_test_piece(board, piece->config, piece->x, piece->y + 1, piece->rot)) {
            // We can move down.
            piece->y += 1;
        } else {
            // We can't move down, lock the piece.
            board_lock_piece(board, piece->config, piece->x, piece->y, piece->rot);

            // Clear the board of any lines.
            uint8_t lines = board_clear_lines(board);

            // Advance the new piece.
            if (!board_next_piece(board)) {
                return STATE_RESULT_GAMEOVER;
            }
            audio_playsound(g_sound_piece0);

            // Get the piece pointer again, because we mutated it.
            piece = board->piece;
        }
    }

    // Handle movement.
    //
    // Here we track the number of frames we've been holding a particular
    // direction.  We use this to track DAS and to also ensure that pressing
    // both directions at once in a staggered way behaves correctly.
    if (events & EVENT_LEFT) {
        if (state->playstates[0].left_tic == 0) {
            state->playstates[0].left_tic = state->tic;
        }
    } else {
        state->playstates[0].left_tic = 0;
    }
    if (events & EVENT_RIGHT) {
        if (state->playstates[0].right_tic == 0) {
            state->playstates[0].right_tic = state->tic;
        }
    } else {
        state->playstates[0].right_tic = 0;
    }

    int8_t dx = 0;
    if (state->playstates[0].left_tic == state->playstates[0].right_tic) {
        // Either neither event is happening, or both events started at once.
    } else if (state->playstates[0].left_tic > state->playstates[0].right_tic) {
        // Ignore right event, figure out our left event DAS.
        uint32_t tics = state->tic - state->playstates[0].left_tic;
        if (tics == 0) {
            // Move immediately
            dx = -1;
        } else if (tics >= DEFAULT_DAS) {
            // Waited out the delay.
            tics -= DEFAULT_DAS;
            if (DEFAULT_DAS_PERIOD == 0) {
                dx = -(state->boards[0]->config.width);
            } else if  (tics % DEFAULT_DAS_PERIOD == 0) {
                dx = -1;
            }
        }
    } else if (state->playstates[0].left_tic < state->playstates[0].right_tic) {
        // Ignore left event, figure out our right event DAS.
        uint32_t tics = state->tic - state->playstates[0].right_tic;
        if (tics == 0) {
            // Move immediately.
            dx = 1;
        } else if (tics >= DEFAULT_DAS) {
            // Waited out the delay.
            tics -= DEFAULT_DAS;
            if (DEFAULT_DAS_PERIOD == 0) {
                dx = state->boards[0]->config.width;
            } else if  (tics % DEFAULT_DAS_PERIOD == 0) {
                dx = 1;
            }
        }
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
        if (state->playstates[0].ccw_already == false) {
            // Only rotate if this is the first tic of the event
            drot -= 1;
            state->playstates[0].ccw_already = true;
        }
    } else {
        state->playstates[0].ccw_already = false;
    }
    if (events & EVENT_CW) {
        if (state->playstates[0].cw_already == false) {
            // Only rotate if this is the first tic of the event
            drot += 1;
            state->playstates[0].cw_already = true;
        }
    } else {
        state->playstates[0].cw_already = false;
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
            // Normal rotation.
            piece->rot = prot;
        } else if (piece->config == &g_o_piece) {
            // Don't wallkick the "O" piece.
        } else if (piece->config == &g_i_piece) {
            // Wallkicks for the "I" piece are unique.
            int8_t tries_x[4] = { 0 };
            int8_t tries_y[4] = { 0 };

            if ((piece->rot == ROT_0 && prot == ROT_L) || (piece->rot == ROT_R && prot == ROT_2)) {
                tries_x[0] = -1; tries_y[0] = 0;
                tries_x[1] = 2; tries_y[1] = 0;
                tries_x[2] = -1; tries_y[2] = -2;
                tries_x[3] = 2; tries_y[3] = 1;
            } else if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_L && prot == ROT_2)) {
                tries_x[0] = -2; tries_y[0] = 0;
                tries_x[1] = 1; tries_y[1] = 0;
                tries_x[2] = -2; tries_y[2] = 1;
                tries_x[3] = 1; tries_y[3] = -2;
            } else if ((piece->rot == ROT_2 && prot == ROT_R) || (piece->rot == ROT_L && prot == ROT_0)) {
                tries_x[0] = 1; tries_y[0] = 0;
                tries_x[1] = -2; tries_y[1] = 0;
                tries_x[2] = 1; tries_y[2] = 2;
                tries_x[3] = -2; tries_y[3] = -1;
            } else /* ROT_2 -> ROT_L, ROT_R -> ROT_0 */ {
                tries_x[0] = 2; tries_y[0] = 0;
                tries_x[1] = -1; tries_y[1] = 0;
                tries_x[2] = 2; tries_y[2] = -1;
                tries_x[3] = -1; tries_y[3] = 2;
            }

            for (size_t i = 0;i < piece->config->data_count;i++) {
                if (board_test_piece(board, piece->config,
                        piece->x + tries_x[i], piece->y + tries_y[i], prot)) {
                    piece->x += tries_x[i];
                    piece->y += tries_y[i];
                    piece->rot = prot;
                    break;
                }
            }
        } else {
            // Wallkicks for the other pieces.
            int8_t tries_x[4] = { 0 };
            int8_t tries_y[4] = { 0 };

            if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_2 && prot == ROT_R)) {
                tries_x[0] = -1; tries_y[0] = 0;
                tries_x[1] = -1; tries_y[1] = -1;
                tries_x[2] = 0; tries_y[2] = 2;
                tries_x[3] = -1; tries_y[3] = 2;
            } else if ((piece->rot == ROT_L && prot == ROT_2) || (piece->rot == ROT_L && prot == ROT_0)) {
                tries_x[0] = -1; tries_y[0] = 0;
                tries_x[1] = -1; tries_y[1] = 1;
                tries_x[2] = 0; tries_y[2] = -2;
                tries_x[3] = -1; tries_y[3] = -2;
            } else if ((piece->rot == ROT_R && prot == ROT_0) || (piece->rot == ROT_R && prot == ROT_2)) {
                tries_x[0] = 1; tries_y[0] = 0;
                tries_x[1] = 1; tries_y[1] = 1;
                tries_x[2] = 0; tries_y[2] = -2;
                tries_x[3] = 1; tries_y[3] = -2;
            } else /* ROT_2 -> ROT_L, ROT_0 -> ROT_L */ {
                tries_x[0] = 1; tries_y[0] = 0;
                tries_x[1] = 1; tries_y[1] = -1;
                tries_x[2] = 0; tries_y[2] = 2;
                tries_x[3] = 1; tries_y[3] = 2;
            }

            for (size_t i = 0;i < piece->config->data_count;i++) {
                if (board_test_piece(board, piece->config,
                        piece->x + tries_x[i], piece->y + tries_y[i], prot)) {
                    piece->x += tries_x[i];
                    piece->y += tries_y[i];
                    piece->rot = prot;
                    break;
                }
            }
        }
    }

    // We're done with all processing for this tic.
    return STATE_RESULT_OK;
}
