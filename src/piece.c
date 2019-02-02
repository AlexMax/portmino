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

#include "frontend.h"
#include "piece.h"

/**
 * Allocates a piece configuration array table at the top of the Lua state
 * 
 * Assumes you have the table of pieces on the top of the Lua stack.  Consumes
 * the table from the Lua stack and leaves nothing on success, or an error
 * message on failure.
 */
piece_configs_t* piece_configs_new(lua_State* L) {
    // Take note of the top of the stack.
    int top = lua_gettop(L);

    // Push our available pieces into the ruleset.
    lua_Integer pieces_length = luaL_len(L, -1);
    if (pieces_length <= 0) {
        lua_settop(L, top - 1);
        lua_pushstring(L, "No pieces defined");
        return NULL;
    } else if (pieces_length > MAX_PIECES) {
        lua_settop(L, top - 1);
        lua_pushstring(L, "Too many pieces defined");
        return NULL;
    }

    piece_configs_t* piece_configs = calloc(1, sizeof(piece_configs_t));
    if (piece_configs == NULL) {
        lua_pushstring(L, "Allocation error");
        return NULL;
    }

    piece_configs->configs = calloc(pieces_length, sizeof(piece_config_t*));
    if (piece_configs->configs == NULL) {
        piece_configs_delete(piece_configs);
        lua_settop(L, top - 1);
        lua_pushstring(L, "Allocation error");
        return NULL;
    }
    piece_configs->size = pieces_length;
    for (lua_Integer i = 1;i <= pieces_length;i++) {
        // Get the individual piece table
        int type = lua_rawgeti(L, -1, i);
        if (type != LUA_TTABLE) {
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Piece definition isn't a table");
            return NULL;
        }

        /// Get the various fields of the piece configuration
        lua_pushstring(L, "name");
        lua_gettable(L, -2);
        const char* cname = lua_tostring(L, -1);
        if (cname == NULL) {
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Piece \"name\" isn't a valid string");
            return NULL;
        }
        char* name = strdup(cname);
        lua_pop(L, 1); // pop name

        // Spawn Position is a tuple of integers that contain an x and y coordinate
        lua_pushstring(L, "spawn_pos");
        type = lua_gettable(L, -2);
        if (type != LUA_TTABLE) {
            free(name);
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Piece \"spawn_pos\" isn't a table");
            return NULL;
        }

        lua_Integer spawn_pos_length = luaL_len(L, -1);
        if (spawn_pos_length != 2) {
            free(name);
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Piece \"spawn_pos\" needs exactly two coordinates");
            return NULL;
        }

        vec2i_t spawn_pos = vec2i_zero();
        lua_rawgeti(L, -1, 1);
        spawn_pos.x = (int)lua_tointeger(L, -1);
        lua_pop(L, 1); // pop x

        lua_rawgeti(L, -1, 2);
        spawn_pos.y = (int)lua_tointeger(L, -1);
        lua_pop(L, 1); // pop y
        lua_pop(L, 1); // pop spawn_pos

        lua_pushstring(L, "spawn_rot");
        lua_gettable(L, -2);
        uint8_t spawn_rot = (uint8_t)lua_tointeger(L, -1);
        lua_pop(L, 1); // pop spawn_rot

        lua_pushstring(L, "width");
        lua_gettable(L, -2);
        uint8_t width = (uint8_t)lua_tointeger(L, -1);
        lua_pop(L, 1); // pop spawn_rot

        lua_pushstring(L, "height");
        lua_gettable(L, -2);
        uint8_t height = (uint8_t)lua_tointeger(L, -1);
        lua_pop(L, 1); // pop spawn_rot

        // Data is a contiguous array of integers
        lua_pushstring(L, "data");
        type = lua_gettable(L, -2);
        if (type != LUA_TTABLE) {
            free(name);
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Piece \"data\" isn't a table");
            return NULL;
        }

        lua_Integer data_length = luaL_len(L, -1);
        uint8_t* data = calloc(data_length, sizeof(uint8_t));
        if (data == NULL) {
            free(name);
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Allocation error");
            return NULL;
        }

        // Grab our array members.  Lua is 1-indexed.
        for (lua_Integer i = 1;i <= data_length;i++) {
            lua_rawgeti(L, -1, i);
            data[i - 1] = (uint8_t)lua_tointeger(L, -1);
            lua_pop(L, 1);
        }
        lua_pop(L, 1); // pop data

        lua_pop(L, 1); // pop piece table

        piece_config_t* piece_config = calloc(1, sizeof(piece_config_t));
        if (piece_config == NULL) {
            free(name);
            free(data);
            piece_configs_delete(piece_configs);
            lua_settop(L, top - 1);
            lua_pushstring(L, "Allocation error");
            return NULL;
        }

        piece_config->name = name;
        piece_config->datas = data;
        piece_config->spawn_pos = spawn_pos;
        piece_config->spawn_rot = spawn_rot;
        piece_config->width = width;
        piece_config->height = height;
        piece_config->data_size = width * height * sizeof(piece_config->datas[0]);
        piece_config->data_count = data_length / piece_config->data_size;

        piece_configs->configs[i - 1] = piece_config;
    }

    lua_settop(L, top - 1);
    return piece_configs;
}

/**
 * Frees a piece configuration array
 */
void piece_configs_delete(piece_configs_t* piece_configs) {
    if (piece_configs->configs != NULL) {
        if (piece_configs->size != 0) {
            for (size_t i = 0;i < piece_configs->size;i++) {
                if (piece_configs->configs[i] != NULL) {
                    free(piece_configs->configs[i]->name);
                    piece_configs->configs[i]->name = NULL;
                    free(piece_configs->configs[i]->datas);
                    piece_configs->configs[i]->datas = NULL;
                    free(piece_configs->configs[i]);
                    piece_configs->configs[i] = NULL;
                }
            }
        }

        free(piece_configs->configs);
        piece_configs->configs = NULL;
    }

    free(piece_configs);
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
    return piece->datas + (rot * piece->data_size);
}
