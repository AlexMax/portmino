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

#include <lua.h>

#include "error.h"
#include "ruleset.h"

/**
 * Actually call the next piece function
 *
 * Returns a piece configuration
 */
static piece_config_t* next_next_piece(next_t* next) {
    // Find our next_piece function.
    int type = lua_rawgeti(next->lua, LUA_REGISTRYINDEX, next->next_piece_ref);
    if (type != LUA_TFUNCTION) {
        error_push("Unable to find next_piece function.");
        return NULL;
    }

    // Call it.
    if (lua_pcall(next->lua, 0, 1, 0) != LUA_OK) {
        const char* err = lua_tostring(next->lua, -1);
        error_push("lua error: %s", err);
        return NULL;
    }

    // We expect to get a light userdata out of it.  Complain if we don't.
    if (lua_type(next->lua, -1) != LUA_TLIGHTUSERDATA) {
        error_push("Unknown return value of next_piece function.");
        return NULL;
    }

    // Return the piece.
    piece_config_t* piece = lua_touserdata(next->lua, -1);
    return piece;
}

/**
 * Allocate a new next piece buffer.
 *
 * The next struct takes ownership of the "next piece" function reference.
 * The caller is not expected to unref it.
 */
bool next_init(next_t* next, lua_State* L, int next_piece_ref) {
    memset(next, 0x00, sizeof(next_t));

    next->lua = L;
    next->next_piece_ref = next_piece_ref;
    next->next_index = 0;

    for (size_t i = 0;i < MAX_NEXTS;i++) {
        next->nexts[i] = next_next_piece(next);
        if (next->nexts[i] == NULL) {
            error_push("Cannot set a next piece.");
            next_deinit(next);
            return NULL;
        }
    }

    return next;
}

/**
 * Delete the next piece buffer.
 */
void next_deinit(next_t* next) {
    if (next == NULL) {
        return;
    }

    for (size_t i = 0;i < MAX_NEXTS;i++) {
        next->nexts[i] = NULL;
    }

    memset(next, 0x00, sizeof(next_t));
}

/**
 * Return the configuration of the next piece
 */
const piece_config_t* next_get_next_piece(const next_t* next, size_t index) {
    index = next->next_index + index % MAX_NEXTS;
    return next->nexts[index];
}

/**
 * "Consume" the next piece
 * 
 * Advances the next-piece index, wrapping around if necessary, and generates
 * a new next piece at the end.
 */
void next_consume_next_piece(next_t* next) {
    next->nexts[next->next_index] = next_next_piece(next);
    next->next_index = (next->next_index + 1) % MAX_NEXTS;
}
