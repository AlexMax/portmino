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

#include "event.h"
#include "field.h"
#include "state.h"

typedef struct {
    /**
     * Current gametic
     */
    uint32_t tic;

    /**
     * Playfield
     */
    field_t* field;
} state_t;

state_t* state_new(void);
void state_delete(state_t* state);
void state_frame(state_t* state, events_t events);
void state_debug(state_t* state);
