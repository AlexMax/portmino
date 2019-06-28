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

// Forward declarations
typedef struct lua_State lua_State;

typedef struct serialize_s {
    /**
     * Lua instance used by serialization.
     */
    lua_State* lua;

    /**
     * Registry reference used by serialization.
     */
    int registry_ref;
} serialize_t;

buffer_t* serialize_to_serialized(serialize_t* ser, int index);
void serialize_push_serialized(serialize_t* ser, const buffer_t* buffer);
