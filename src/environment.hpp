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

#include "input.hpp"

// Forward declarations.
typedef struct lua_State lua_State;
typedef struct proto_container_s proto_container_t;

/**
 * Portable state - everything needed to get back to a previous state.
 */
typedef struct portstate_s {
    /**
     * Serialized state
     */
    buffer_t* serialized;

    /**
     * Next entity ID
     */
    uint32_t entity_next;

    /**
     * Gametic of serialized state
     */
    uint32_t gametic;
} portstate_t;

typedef struct environment_s {
    /**
     * Pointer to lua state.
     *
     * This pointer is not owned by this structure.  Do not free it.
     */
    lua_State* lua;

    /**
     * Reference to registry.
     */
    int registry_ref;

    /**
     * Reference to environment.
     */
    int env_ref;

    /**
     * Reference to ruleset module.
     */
    int ruleset_ref;

    /**
     * Reference to state table.
     */
    int state_ref;

    /**
     * Last processed tic.
     */
    uint32_t gametic;

    /**
     * Prototypes loaded from Lua.
     */
    proto_container_t* protos;

    /**
     * Serialized states.
     */
    portstate_t states[4];
} environment_t;

environment_t* environment_new(lua_State* L, const char* ruleset, const char* gametype);
void environment_delete(environment_t* env);
bool environment_dostring(environment_t* env, const char* script);
bool environment_start(environment_t* env);
bool environment_save(environment_t* env);
bool environment_rewind(environment_t* env, uint32_t frame);
bool environment_frame(environment_t* env, const playerinputs_t* inputs);
void environment_draw(environment_t* env);
