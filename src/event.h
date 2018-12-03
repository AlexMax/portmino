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

#include <stdint.h>

/**
 * An integer type that has enough room to contain all bits of the event
 * bitfield.  If even a single bit is added, we need to expand to 16-bits.
 */
typedef uint8_t events_t;

/**
 * Event bits.
 */
enum {
    EVENT_NONE = 0,
    EVENT_LEFT = 1,
    EVENT_RIGHT = 1 << 1,
    EVENT_SOFTDROP = 1 << 2,
    EVENT_HARDDROP = 1 << 3,
    EVENT_CCW = 1 << 4,
    EVENT_CW = 1 << 5,
    EVENT_HOLD = 1 << 6,
    EVENT_180 = 1 << 7,
};
