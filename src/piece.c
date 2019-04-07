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

#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"

#include "error.h"
#include "frontend.h"
#include "piece.h"
#include "script.h"

/**
 * Allocates a piece configuration from the table at the top of the Lua stack
 * 
 * Assumes you have a piece configuration table on the top of the Lua stack.
 * Consumes the table from the Lua stack and leaves nothing on success, or
 * an error message on failure.
 */
piece_config_t* piece_config_new(lua_State* L, const char* name) {
    int top = lua_gettop(L);

    const char* error = NULL;
    char* namedup = NULL;
    piece_config_t* piece = NULL;
    vec2i_t spawn_pos = vec2i_zero();

    // Duplicate our name string
    if ((namedup = strdup(name)) == NULL) {
        error = "Allocation error";
        goto fail;
    }

    // Spawn Position is a tuple of integers that contain an x and y coordinate
    lua_getfield(L, -1, "spawn_pos");
    if (script_to_vector(L, -1, &spawn_pos) == false) {
        error = "Piece \"spawn_pos\" isn't a table";
        goto fail;
    }
    lua_pop(L, 1); // pop spawn_pos

    lua_getfield(L, -1, "spawn_rot");
    uint8_t spawn_rot = (uint8_t)lua_tointeger(L, -1);
    lua_pop(L, 1); // pop spawn_rot

    lua_getfield(L, -1, "width");
    uint8_t width = (uint8_t)lua_tointeger(L, -1);
    lua_pop(L, 1); // pop width

    lua_getfield(L, -1, "height");
    uint8_t height = (uint8_t)lua_tointeger(L, -1);
    lua_pop(L, 1); // pop height

    // Data is a contiguous array of integers
    int type = lua_getfield(L, -1, "data");
    if (type != LUA_TTABLE) {
        error = "Piece \"data\" isn't a table";
        goto fail;
    }

    lua_Integer data_length = luaL_len(L, -1);
    uint8_t* data = calloc(data_length, sizeof(uint8_t));
    if (data == NULL) {
        error = "Allocation error";
        goto fail;
    }

    // Grab our array members.
    for (lua_Integer i = 1;i <= data_length;i++) {
        lua_rawgeti(L, -1, i);
        data[i - 1] = (uint8_t)lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop data
    lua_pop(L, 1); // pop piece table

    piece = calloc(1, sizeof(piece_config_t));
    if (piece == NULL) {
        error_push_allocerr();
        goto fail;
    }

    piece->name = namedup;
    piece->data = data;
    piece->spawn_pos = spawn_pos;
    piece->spawn_rot = spawn_rot;
    piece->width = width;
    piece->height = height;
    piece->data_size = width * height * sizeof(piece->data[0]);
    piece->data_count = data_length / piece->data_size;

    return piece;

fail:
    lua_settop(L, top); // reset stack to previous position
    lua_pushstring(L, error); // push error
    piece_config_delete(piece);
    free(namedup);
    return NULL;
}

/**
 * Frees a piece configuration
 */
void piece_config_delete(piece_config_t* piece_config) {
    if (piece_config == NULL) {
        return;
    }

    free(piece_config->name);
    piece_config->name = NULL;
    free(piece_config->data);
    piece_config->data = NULL;

    free(piece_config);
}

/**
 * A generic destructor for the piece configuration
 */
void piece_config_destruct(void* piece_config) {
    piece_config_delete((piece_config_t*)piece_config);
}

/**
 * Initialize a new piece on the board.
 */
piece_t* piece_new(const piece_config_t* config) {
    piece_t* piece = malloc(sizeof(piece_t));

    piece->config = config;
    piece->rot = config->spawn_rot;
    piece->pos = config->spawn_pos;

    return piece;
}

/**
 * Delete a piece on the board.
 */
void piece_delete(piece_t* piece) {
    free(piece);
}

/**
 * Get the initial data position of a particular rotation.
 */
uint8_t* piece_get_rot(const piece_config_t* piece, uint8_t rot) {
    return piece->data + (rot * piece->data_size);
}
