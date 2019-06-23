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

#include "input.hpp"

#include <string.h>

/**
 * Init the hold struct
 * 
 * We initialize everything to being set, because we don't want to count
 * any keys held before initialization as being set.
 */
void input_holds_init(gameholds_t* holds) {
    memset(holds, 0xFF, sizeof(*holds));
}

/**
 * A sensible delayed auto interface input filter
 */
playerinputs_t input_interface_filter(gameholds_t* holds, const gameinputs_t* inputs) {
    playerinputs_t res = { 0 };

    for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
        inputs_t current = inputs->interface.inputs[i];
        inputs_t last = holds->holds[i].interface.last;
        holds->holds[i].interface.last = current;

        // XOR only sets changed bits, AND ensures we only set bits that
        // were newly set and not newly unset.
        res.inputs[i] = (last ^ current) & current;
    }

    return res;
}

/**
 * A sensible delayed auto menu input filter
 */
playerinputs_t input_menu_filter(gameholds_t* holds, const gameinputs_t* inputs) {
    playerinputs_t res = { 0 };

    for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
        inputs_t current = inputs->menu.inputs[i];
        inputs_t last = holds->holds[i].menu.last;
        holds->holds[i].menu.last = current;

        // XOR only sets changed bits, AND ensures we only set bits that
        // were newly set and not newly unset.
        res.inputs[i] = (last ^ current) & current;
    }

    return res;
}
