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

/**
 * A single event that acts upon the game state
 */
typedef int event_t;

/**
 * Possible event types
 */
enum {
    EVENT_NONE,
    EVENT_LEFT,
    EVENT_RIGHT,
    EVENT_SOFTDROP,
    EVENT_HARDDROP,
    EVENT_CCW,
    EVENT_CW,
    EVENT_HOLD,
    EVENT_180,
    MAX_EVENTS
};

const char* event_to_string(int event);
