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

// Forward declarations.
typedef struct lua_State lua_State;
typedef struct ruleset_s ruleset_t;
typedef struct state_s state_t;
typedef struct vfile_s vfile_t;

typedef struct gametype_s {
    /**
     * Lua interpreter state
     * 
     * This is not an owning reference.  Do not free it.
     */
    lua_State* lua;

    /**
     * Name of the gametype
     */
    char* name;

    /**
     * Init function reference.
     */
    int init_ref;

    /**
     * State functions reference.
     */
    int state_functions_ref;

    /**
     * Draw reference.
     */
    int draw_ref;
} gametype_t;

vfile_t* gametype_find_script(const char* ruleset_name, const char* name);
gametype_t* gametype_new(lua_State* L, buffer_t* file, const char* name);
void gametype_delete(gametype_t* gametype);
bool gametype_initialize(gametype_t* gametype, state_t* state);
