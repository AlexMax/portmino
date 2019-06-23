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

#include "define.hpp"

// Forward declarations.
typedef struct lua_State lua_State;
typedef struct vfile_s vfile_t;

lua_State* script_newstate(void);
bool script_to_vector(lua_State* L, int index, vec2i_t* vec);
void script_push_vector(lua_State* L, const vec2i_t* vec);
void script_wrap_cfuncs(lua_State* L, int index);
bool script_load_config(lua_State* L, vfile_t* file);
void script_push_paths(lua_State* L, const char* ruleset, const char* gametype);
void script_push_cpaths(lua_State* L, const char* ruleset, const char* gametype);
void script_debug(lua_State* L, int index);
void script_debug_stack(lua_State* L);
void script_debug_table(lua_State* L, int index);

