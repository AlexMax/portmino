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

#include "event.h"

#include <string.h>

/**
 * Reset the hold struct.
 */
void event_holds_reset(gameholds_t* holds) {
    memset(holds, 0, sizeof(*holds));
}

/**
 * A sensible delayed auto interface event filter
 */
playerevents_t event_interface_filter(gameholds_t* holds, const gameevents_t* events) {
    playerevents_t res = { 0 };

    for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
        events_t current = events->interface.events[i];
        events_t last = holds->holds[i].interface.last;
        holds->holds[i].interface.last = current;

        // XOR only sets changed bits, AND ensures we only set bits that
        // were newly set and not newly unset.
        res.events[i] = (last ^ current) & current;
    }

    return res;
}

/**
 * A sensible delayed auto menu event filter
 */
playerevents_t event_menu_filter(gameholds_t* holds, const gameevents_t* events) {
    playerevents_t res = { 0 };

    for (size_t i = 0;i < MINO_MAX_PLAYERS;i++) {
        events_t current = events->menu.events[i];
        events_t last = holds->holds[i].menu.last;
        holds->holds[i].menu.last = current;

        // XOR only sets changed bits, AND ensures we only set bits that
        // were newly set and not newly unset.
        res.events[i] = (last ^ current) & current;
    }

    return res;
}
