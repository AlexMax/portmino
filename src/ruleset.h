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

#include "lua.h"

#include "event.h"
#include "state.h"

typedef struct ruleset_s {
    /**
     * Lua interpreter state
     */
    lua_State* lua;

    /**
     * Reference to state_frame function inside Lua.
     */
    int state_frame_ref;

    /**
     * Reference to state function inside Lua.
     */
    int state_ref;
} ruleset_t;

ruleset_t* ruleset_new(void);
void ruleset_delete(ruleset_t* ruleset);
state_result_t ruleset_state_frame(ruleset_t* ruleset, state_t* state,
                                const playerevents_t* playerevents);
