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

bool script_to_vector(lua_State* L, int index, vec2i_t* vec);
void script_push_vector(lua_State* L, const vec2i_t* vec);
bool script_load_config(lua_State* L, buffer_t* file, const char* name);
void script_debug(lua_State* L, int index);
void script_debug_stack(lua_State* L);
void script_debug_table(lua_State* L, int index);
