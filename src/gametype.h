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
     * Gametype label (from config)
     */
    char* label;

    /**
     * Gametype help (from config)
     */
    char* help;
} gametype_t;

gametype_t* gametype_new(lua_State* L, ruleset_t* ruleset, const char* name);
void gametype_delete(gametype_t* gametype);
