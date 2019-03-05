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

/**
 * Maximum number of players that we can accept inputs from simultaneously.
 */
#define MINO_MAX_PLAYERS 2

/**
 * An integer type that has enough room to contain all bits of the input
 * bitfield.  If even a single bit is added, we need to expand to 16-bits.
 */
typedef uint8_t inputs_t;

/**
 * Input bits.
 *
 * These inputs have a direct impact on gamestate.
 */
typedef enum {
    INPUT_NONE = 0,
    INPUT_LEFT = 1,
    INPUT_RIGHT = 1 << 1,
    INPUT_SOFTDROP = 1 << 2,
    INPUT_HARDDROP = 1 << 3,
    INPUT_CCW = 1 << 4,
    INPUT_CW = 1 << 5,
    INPUT_HOLD = 1 << 6,
    INPUT_180 = 1 << 7,
} input_t;

/**
 * Interface Input bits
 *
 * These inputs occur when you're ingame, but are not actions you perform
 * on gamestate per se.
 */
typedef enum {
    IINPUT_NONE = 0,
    IINPUT_PAUSE = 1,
    IINPUT_RESTART = 1 << 1,
    IINPUT_MAINMENU = 1 << 2,
    IINPUT_QUIT = 1 << 3
} iinput_t;

/**
 * Menu Input bits.
 */
typedef enum {
    MINPUT_NONE = 0,
    MINPUT_UP = 1,
    MINPUT_DOWN = 1 << 1,
    MINPUT_LEFT = 1 << 2,
    MINPUT_RIGHT = 1 << 3,
    MINPUT_OK = 1 << 4,
    MINPUT_CANCEL = 1 << 5,
} minput_t;

/**
 * A set of inputs for all players.
 */
typedef struct playerinputs_s {
    inputs_t inputs[MINO_MAX_PLAYERS];
} playerinputs_t;

/**
 * Complete set of all subsystem inputs.
 *
 * To simplify front-end implementation, all "raw" inputs are translated to
 * inputs for all subsystems at once, and the core of portmino decides which
 * sets of inputs are relevent where.
 */
typedef struct gameinputs_s {
    playerinputs_t game;
    playerinputs_t interface;
    playerinputs_t menu;
} gameinputs_t;

/**
 * Keeps track of how long an input has been "held".
 */
typedef struct {
    struct iholds_s {
        inputs_t last;
    } interface;
    struct mholds_s {
        inputs_t last;
    } menu;
} holds_t;

/**
 * Complete set of all subsystem holds.
 * 
 * We don't need to keep track of held tics for the game, as it's up to the
 * game itself how it wants to handle DAS.
 */
typedef struct {
    holds_t holds[MINO_MAX_PLAYERS];
} gameholds_t;

void input_holds_init(gameholds_t* holds);
playerinputs_t input_interface_filter(gameholds_t* holds, const gameinputs_t* inputs);
playerinputs_t input_menu_filter(gameholds_t* holds, const gameinputs_t* inputs);
