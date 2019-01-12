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
 * Maximum number of players that we can accept events from simultaneously.
 */
#define MINO_MAX_PLAYERS 2

/**
 * An integer type that has enough room to contain all bits of the event
 * bitfield.  If even a single bit is added, we need to expand to 16-bits.
 */
typedef uint8_t events_t;

/**
 * Event bits.
 *
 * These events have a direct impact on gamestate.
 */
typedef enum {
    EVENT_NONE = 0,
    EVENT_LEFT = 1,
    EVENT_RIGHT = 1 << 1,
    EVENT_SOFTDROP = 1 << 2,
    EVENT_HARDDROP = 1 << 3,
    EVENT_CCW = 1 << 4,
    EVENT_CW = 1 << 5,
    EVENT_HOLD = 1 << 6,
    EVENT_180 = 1 << 7,
} event_t;

/**
 * Interface Event bits
 *
 * These events occur when you're ingame, but are not actions you perform
 * on gamestate per se.
 */
typedef enum {
    IEVENT_NONE = 0,
    IEVENT_PAUSE = 1,
    IEVENT_RESTART = 1 << 1,
    IEVENT_MAINMENU = 1 << 2,
    IEVENT_QUIT = 1 << 3
} ievent_t;

/**
 * Menu Event bits.
 */
typedef enum {
    MEVENT_NONE = 0,
    MEVENT_UP = 1,
    MEVENT_DOWN = 1 << 1,
    MEVENT_LEFT = 1 << 2,
    MEVENT_RIGHT = 1 << 3,
    MEVENT_OK = 1 << 4,
    MEVENT_CANCEL = 1 << 5,
} mevent_t;

/**
 * A set of events for all players.
 */
typedef struct {
    events_t events[MINO_MAX_PLAYERS];
} playerevents_t;

/**
 * Complete set of all subsystem events.
 *
 * To simplify front-end implementation, all inputs are translated to events
 * for all subsystems at once, and the core of portmino decides which sets
 * of events are relevent where.
 */
typedef struct {
    playerevents_t game;
    playerevents_t interface;
    playerevents_t menu;
} gameevents_t;
