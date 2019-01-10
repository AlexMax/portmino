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
#define DEFAULT_LOCK_DELAY 30

/**
 * Reset a player state struct to nothing.
 */
void playstate_reset(playstate_t* ps) {
    ps->left_tic = 0;
    ps->right_tic = 0;
    ps->lock_tic = 0;
    ps->harddrop_tic = 0;
    ps->ccw_already = false;
    ps->cw_already = false;
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

#include <stdio.h>

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

    // Is our piece blocked from the bottom?  If so, lock logic takes priority.
    if (!board_test_piece(board, piece->config, vec2i(piece->pos.x, piece->pos.y + 1), piece->rot)) {
        if (state->playstates[0].lock_tic == 0) {
            // This is our first tic that we've locked.
            state->playstates[0].lock_tic = state->tic;
            audio_playsound(g_sound_step);
        }

        if (state->tic - state->playstates[0].lock_tic >= DEFAULT_LOCK_DELAY) {
            // Our lock timer has run out, lock the piece.
            board_lock_piece(board, piece->config, piece->pos, piece->rot);
            audio_playsound(g_sound_lock);

            // Clear the board of any lines.
            uint8_t lines = board_clear_lines(board);
            (void)lines;

            // Advance the new piece.
            if (!board_next_piece(board)) {
                return STATE_RESULT_GAMEOVER;
            }
            audio_playsound(g_sound_piece0);

            // We're done with locking, so cancel the tic out.
            state->playstates[0].lock_tic = 0;

            // Get the piece pointer again, because we mutated it.
            piece = board->piece;
        }
    } else {
        // We are not in lock logic anymore.
        state->playstates[0].lock_tic = 0;
    }

    // Determine what our gravity is going to be.
    int gravity_tics = 64; // number of tics between gravity tics
    int gravity_cells = 1; // number of cells to move the piece per gravity tics.

    // Soft dropping and hard dropping aren't anything too special, they
    // just toy with gravity.
    if (events & EVENT_SOFTDROP) {
        gravity_tics = 2;
        gravity_cells = 1;
    }

    // If you press soft and hard drop at the same time, hard drop wins.
    // If you hold hard drop and press soft drop afterwards, soft drop wins.
    if (events & EVENT_HARDDROP) {
        if (state->playstates[0].harddrop_tic == 0) {
            // We only pay attention to hard drops on the tic they were invoked.
            // Othewise, you have rapid-fire dropping or even pieces running
            // into each other at the top of the well.
            gravity_tics = 1;
            gravity_cells = 20;

            state->playstates[0].harddrop_tic = state->tic;
        }
    } else {
        state->playstates[0].harddrop_tic = 0;
    }

    // Handle gravity.
    if (state->tic % gravity_tics == 0) {
        vec2i_t src = { piece->pos.x, piece->pos.y };
        vec2i_t dst = { piece->pos.x, piece->pos.y + gravity_cells };
        vec2i_t res = board_test_piece_between(board, piece->config, src, piece->rot, dst);

        // Our new location is always wherever the test tells us.  If we
        // can't move down, we're relying on our lock delay logic to handle
        // things next tic.
        piece->pos.y = res.y;

        if (state->playstates[0].harddrop_tic == state->tic) {
            // ...unless our gravity was actually a hard drop.  In that case,
            // lock the piece immediately
            board_lock_piece(board, piece->config, piece->pos, piece->rot);
            audio_playsound(g_sound_lock);

            // Clear the board of any lines.
            uint8_t lines = board_clear_lines(board);
            (void)lines;

            // Advance the new piece.
            if (!board_next_piece(board)) {
                return STATE_RESULT_GAMEOVER;
            }
            audio_playsound(g_sound_piece0);

            // Doing a hard drop is mutually exclusive with any other piece
            // movement.
            return STATE_RESULT_OK;
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

    // dx will be != depending on where the piece must be moved.
    if (dx != 0) {
        vec2i_t dpos = piece->pos;
        dpos.x += dx;
        if (board_test_piece(board, piece->config, dpos, piece->rot)) {
            piece->pos.x += dx;
            audio_playsound(g_sound_move);

            // Moving the piece successfully resets our lock timer.
            if (state->playstates[0].lock_tic != 0) {
                state->playstates[0].lock_tic = state->tic;
                audio_playsound(g_sound_step);
            }
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

        // Figure out which wallkicks we need to calculate.
        vec2i_t tries[5] = { 0 };
        if (piece->config == &g_o_piece) {
            // Don't wallkick the "O" piece.
        } else if (piece->config == &g_i_piece) {
            // Wallkicks for the "I" piece are unique.
            if ((piece->rot == ROT_0 && prot == ROT_L) || (piece->rot == ROT_R && prot == ROT_2)) {
                tries[1].x = -1; tries[1].y = 0;
                tries[2].x = 2; tries[2].y = 0;
                tries[3].x = -1; tries[3].y = -2;
                tries[4].x = 2; tries[4].y = 1;
            } else if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_L && prot == ROT_2)) {
                tries[1].x = -2; tries[1].y = 0;
                tries[2].x = 1; tries[2].y = 0;
                tries[3].x = -2; tries[3].y = 1;
                tries[4].x = 1; tries[4].y = -2;
            } else if ((piece->rot == ROT_2 && prot == ROT_R) || (piece->rot == ROT_L && prot == ROT_0)) {
                tries[1].x = 1; tries[1].y = 0;
                tries[2].x = -2; tries[2].y = 0;
                tries[3].x = 1; tries[3].y = 2;
                tries[4].x = -2; tries[4].y = -1;
            } else /* ROT_2 -> ROT_L, ROT_R -> ROT_0 */ {
                tries[1].x = 2; tries[1].y = 0;
                tries[2].x = -1; tries[2].y = 0;
                tries[3].x = 2; tries[3].y = -1;
                tries[4].x = -1; tries[4].y = 2;
            }
        } else {
            // Wallkicks for the other pieces.
            if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_2 && prot == ROT_R)) {
                tries[0].x = -1; tries[0].y = 0;
                tries[1].x = -1; tries[1].y = -1;
                tries[2].x = 0; tries[2].y = 2;
                tries[3].x = -1; tries[3].y = 2;
            } else if ((piece->rot == ROT_L && prot == ROT_2) || (piece->rot == ROT_L && prot == ROT_0)) {
                tries[0].x = -1; tries[0].y = 0;
                tries[1].x = -1; tries[1].y = 1;
                tries[2].x = 0; tries[2].y = -2;
                tries[3].x = -1; tries[3].y = -2;
            } else if ((piece->rot == ROT_R && prot == ROT_0) || (piece->rot == ROT_R && prot == ROT_2)) {
                tries[0].x = 1; tries[0].y = 0;
                tries[1].x = 1; tries[1].y = 1;
                tries[2].x = 0; tries[2].y = -2;
                tries[3].x = 1; tries[3].y = -2;
            } else /* ROT_2 -> ROT_L, ROT_0 -> ROT_L */ {
                tries[0].x = 1; tries[0].y = 0;
                tries[1].x = 1; tries[1].y = -1;
                tries[2].x = 0; tries[2].y = 2;
                tries[3].x = 1; tries[3].y = 2;
            }
        }

        // Finally, run our tests.
        for (size_t i = 0;i < piece->config->data_count;i++) {
            vec2i_t test_pos = { 
                piece->pos.x + tries[i].x,
                piece->pos.y + tries[i].y
            };
            if (board_test_piece(board, piece->config, test_pos, prot)) {
                piece->pos.x += tries[i].x;
                piece->pos.y += tries[i].y;
                piece->rot = prot;
                audio_playsound(g_sound_rotate);

                // Rotating the piece successfully resets our lock timer.
                if (state->playstates[0].lock_tic != 0) {
                    state->playstates[0].lock_tic = state->tic;
                    audio_playsound(g_sound_step);
                }

                break;
            }
        }
    }

    // Ghost piece logic.
    if (board->ghost == NULL) {
        // Create a ghost piece.
        board->ghost = piece_new(piece->config);
    } else if (board->ghost->config != board->piece->config) {
        // Update the ghost piece configuration.
        board->ghost->config = board->piece->config;
    }

    vec2i_t ghost_src = board->piece->pos;
    vec2i_t ghost_dst = { board->piece->pos.x, board->config.height };
    vec2i_t ghost_loc = board_test_piece_between(board, board->ghost->config,
        ghost_src, piece->rot, ghost_dst);

    board->ghost->pos = ghost_loc;
    board->ghost->rot = piece->rot;

    // We're done with all processing for this tic.
    return STATE_RESULT_OK;
}
