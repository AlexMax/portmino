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

static const char* event_string[] = {
    "EVENT_NONE",
    "EVENT_LEFT",
    "EVENT_RIGHT",
    "EVENT_SOFTDROP",
    "EVENT_HARDDROP",
    "EVENT_CCW",
    "EVENT_CW",
    "EVENT_HOLD",
    "EVENT_180",
};

/**
 * Turn an event into a visible string.
 * 
 * @param event Event to stringify.
 * @return const char* The stringified event.
 */
const char* event_to_string(event_t event) {
    if (event >= MAX_EVENTS) {
        return "(unknown event)";
    }

    return event_string[event];
}
