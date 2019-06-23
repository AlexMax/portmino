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

// Forward declarations.
typedef struct lua_State lua_State;
typedef struct menulist_s menulist_t;

typedef struct ruleset_s {
    /**
     * Lua interpreter state
     * 
     * This is not an owning reference.  Do not free it.
     */
    lua_State* lua;

    /**
     * Name of the ruleset
     */
    char* name;

    /**
     * Ruleset label (from config)
     */
    char* label;

    /**
     * Ruleset help (from config)
     */
    char* help;
} ruleset_t;

ruleset_t* ruleset_new(lua_State* L, const char* name);
void ruleset_delete(ruleset_t* ruleset);
menulist_t* ruleset_get_gametypes(ruleset_t* ruleset);
