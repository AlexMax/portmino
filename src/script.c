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

#include "script.h"

#include <stdio.h>
#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"
#include "mpack.h"

#include "audioscript.h"
#include "boardscript.h"
#include "error.h"
#include "inputscript.h"
#include "globalscript.h"
#include "piecescript.h"
#include "random.h"
#include "randomscript.h"
#include "renderscript.h"
#include "vfs.h"

// Forward declarations.
static bool serialize_table(lua_State* L, int index, mpack_writer_t* writer);

/**
 * Create a lua state that contains all of our libraries
 */
lua_State* script_newstate(void) {
    lua_State* L = NULL;

    // Create a new Lua state
    if ((L = luaL_newstate()) == NULL) {
        error_push_allocerr();
        return NULL;
    }

    static const luaL_Reg loadedlibs[] = {
        { "_G", globalscript_openlib },
        { "mino_audio", audioscript_openlib },
        { "mino_board", boardscript_openlib },
        { "mino_input", inputscript_openlib },
        { "mino_piece", piecescript_openlib },
        { "mino_random", randomscript_openlib },
        { "mino_render", renderscript_openlib },
        { NULL, NULL }
    };

    // Push our library functions into the new state.
    for (const luaL_Reg* lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }

    return L;
}

/**
 * Turn the table at the given stack index into a vector.
 */
bool script_to_vector(lua_State* L, int index, vec2i_t* vec) {
    if (lua_type(L, index) != LUA_TTABLE) {
        return false;
    }

    lua_getfield(L, index, "x");
    vec->x = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, index, "y");
    vec->y = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return true;
}

/**
 * Push a table to the stack with the contents of the given vector.
 */
void script_push_vector(lua_State* L, const vec2i_t* vec) {
    lua_createtable(L, 2, 0);
    lua_pushinteger(L, vec->x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, vec->y);
    lua_setfield(L, -2, "y");
}

static bool serialize_index(lua_State* L, int index, mpack_writer_t* writer) {
    if (index < 0) {
        // First translate into absolute index
        index = lua_gettop(L) + index + 1;
    }

    // Serialization depends on type
    int type = lua_type(L, index);
    switch (type) {
    case LUA_TNIL:
        mpack_write_nil(writer);
        break;
    case LUA_TNUMBER: {
        if (lua_isinteger(L, index) == 1) {
            mpack_write_i64(writer, lua_tointeger(L, index));
        } else {
            mpack_write_double(writer, lua_tonumber(L, index));
        }
        break;
    }
    case LUA_TBOOLEAN:
        mpack_write_bool(writer, (bool)lua_toboolean(L, index));
        break;
    case LUA_TSTRING: {
        size_t str_len = 0;
        const char* str = lua_tolstring(L, index, &str_len);
        mpack_write_str(writer, str, str_len);
        break;
    }
    case LUA_TTABLE:
        serialize_table(L, index, writer);
        break;
    case LUA_TUSERDATA: {
        random_t* random = luaL_testudata(L, index, "random_t");
        if (random != NULL) {
            // Is this a random struct?  Serialize the random state...
            buffer_t* random_data = random_serialize(random);
            if (random_data == NULL) {
                return false;
            }

            mpack_start_bin(writer, 1 + random_data->size);
            mpack_write_bytes(writer, "\x01", 1);
            mpack_write_bytes(writer, (const char*)(random_data->data), random_data->size);
            mpack_finish_bin(writer);
            break;
        }

        // No clue what this is then...
        error_push("Data is not serializable.");
        return false;
    }
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
        error_push("Data is not serializable.");
        return false;
    }

    return true;
}

static bool serialize_table(lua_State* L, int index, mpack_writer_t* writer) {
    int top = lua_gettop(L);

    if (index < 0) {
        // First translate into absolute index
        index = lua_gettop(L) + index + 1;
    }

    if (lua_type(L, index) != LUA_TTABLE) {
        error_push("Tried to serialize a non-table as a table.");
        return false;
    }

    // If we have an integer key of 1, assume array, otherwise assume table
    lua_rawgeti(L, index, 1);
    if (lua_isnil(L, -1)) {
        // Assume table, figure out length
        uint32_t length = 0;
        while (lua_next(L, index) != 0) {
            if (length == UINT32_MAX) {
                error_push("Length of Lua table too large.");
                goto fail;
            }
            length += 1;
            lua_pop(L, 1);
        }

        // Write out the contents of the map
        mpack_start_map(writer, length);
        lua_pushnil(L);
        while (lua_next(L, index) != 0) {
            size_t str_len = 0;
            const char* str = lua_tolstring(L, -2, &str_len);
            mpack_write_str(writer, str, str_len);
            serialize_index(L, -1, writer);
            lua_pop(L, 1);
        }
        mpack_finish_map(writer);
    } else {
        // Assume array, figure out length
        lua_pop(L, 1);
        lua_Integer len = luaL_len(L, index);
        if (len > UINT32_MAX) {
            error_push("Length of Lua table too large.");
            goto fail;
        }
        uint32_t length = (uint32_t)len;

        // Write out the contents of the array
        mpack_start_array(writer, length);
        for (uint32_t i = 1;i <= len;i++) {
            lua_rawgeti(L, index, i);
            serialize_index(L, -1, writer);
            lua_pop(L, 1);
        }
        mpack_finish_array(writer);
    }

    return true;

fail:
    lua_settop(L, top);
    return false;
}

/**
 * Serialize the data at the given index into a buffer
 *
 * The return value is allocated by this function, and the caller is expected
 * to free it.
 */
buffer_t* script_to_serialized(lua_State* L, int index) {
    buffer_t* msgpack = NULL;

    // Allocate our return buffer
    if ((msgpack = calloc(1, sizeof(buffer_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, (char**)(&msgpack->data), &msgpack->size);
    if (serialize_table(L, index, &writer) == false) {
        goto fail;
    }
    if (mpack_writer_destroy(&writer) != mpack_ok) {
        error_push("Serialization error.");
        goto fail;
    }

    return msgpack;

fail:
    buffer_delete(msgpack);
    return NULL;
}

/**
 * Take a configuration file and push a table that contains all defined
 * configuration values to the stack, or an error message if there was a
 * problem parsing the file
 * 
 * Configuration files are just Lua scripts run in the context of an empty
 * environment.
 */
bool script_load_config(lua_State* L, vfile_t* file) {
    // Load the file into Lua as a chunk.
    if (luaL_loadbufferx(L, (char*)file->data, file->size, file->filename, "t") != LUA_OK) {
        // We have an error on the stack, just return it.
        return false;
    }

    // Our environment is an empty table
    lua_newtable(L);
    lua_pushvalue(L, -1); // copy env ptr
    lua_setupvalue(L, -3, 1); // set environment to env pointer, pops env ptr

    // Move the chunk to the top of the stack and call it.
    lua_insert(L, -2);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        // Right now we have an error message _and_ our useless environment
        // pointer.  Get rid of the environment and return just the error.
        lua_insert(L, -1);
        lua_pop(L, 1);
        return false;
    }

    // We have the pointer to our environment at the top of the stack.
    return true;
}

/**
 * Set our package search paths based on our current ruleset and gametype
 * 
 * Note that gametype can be NULL, and both gametype and ruleset can be NULL.
 */
void script_push_paths(lua_State* L, const char* ruleset, const char* gametype) {
    if (gametype != NULL) {
        // Our search path contains gametype and ruleset
        lua_pushfstring(L, 
            "gametype/%s/?.lua;" "gametype/%s/?/init.lua;" \
            "ruleset/%s/?.lua;" "ruleset/%s/?/init.lua;" \
            "script/?.lua;" "script/?/init.lua",
            gametype, gametype, ruleset, ruleset);
    } else if (ruleset != NULL) {
        // Our search path contains ruleset
        lua_pushfstring(L, 
            "ruleset/%s/?.lua;" "ruleset/%s/?/init.lua;" \
            "script/?.lua;" "script/?/init.lua",
            ruleset, ruleset);
    } else {
        // Our search path just has script information
        lua_pushstring(L, "script/?.lua;" "script/?/init.lua");
    }
}

/**
 * Set our config (not C library) search paths based on our current
 * ruleset and gametype
 * 
 * Note that gametype can be NULL, and both gametype and ruleset can be NULL.
 */
void script_push_cpaths(lua_State* L, const char* ruleset, const char* gametype) {
    if (gametype != NULL) {
        // Our search path contains gametype and ruleset
        lua_pushfstring(L,
            "gametype/%s/?.cfg;" "ruleset/%s/?.cfg;" "script/?.cfg;",
            gametype, ruleset);
    } else if (ruleset != NULL) {
        // Our search path contains ruleset
        lua_pushfstring(L,
            "ruleset/%s/?.cfg;" "script/?.cfg", ruleset);
    } else {
        // Our search path just has script information
        lua_pushstring(L, "script/?.cfg");
    }
}

/**
 * Dump the contents of a specific stack index
 */
void script_debug(lua_State* L, int index) {
    int type = lua_type(L, index);
    fprintf(stderr, "(%s) ", lua_typename(L, type));
    switch (type) {
    case LUA_TNIL:
        fprintf(stderr, "nil");
        break;
    case LUA_TBOOLEAN:
        fprintf(stderr, "%s", lua_toboolean(L, index) ? "true" : "false");
        break;
    case LUA_TNUMBER:
        fprintf(stderr, "%f", lua_tonumber(L, index));
        break;
    case LUA_TSTRING:
        fprintf(stderr, "%s", lua_tostring(L, index));
        break;
    default:
        break;
    }
}

/**
 * Dump the contents of the stack
 */
void script_debug_stack(lua_State* L) {
    int top = lua_gettop(L);
    if (top == 0) {
        fprintf(stderr, "Stack is empty...\n");
        return;
    }

    for (int i = 1;i <= top;i++) {
        fprintf(stderr, "[%d] ", i);
        script_debug(L, i);
        fprintf(stderr, "\n");
    }
}

/**
 * Dump the contents of a table at a specific index
 */
void script_debug_table(lua_State* L, int index) {
    if (index < 0) {
        // First translate into absolute index
        index = lua_gettop(L) + index + 1;
    }

    if (lua_type(L, index) != LUA_TTABLE) {
        fputs("not a table\n", stderr);
        return;
    }

    bool empty = true;
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        empty = false;
        script_debug(L, -2); // key
        fputs(", ", stderr);
        script_debug(L, -1); // value
        fputs("\n", stderr);
        lua_pop(L, 1); // keep the key around
    }

    if (empty == true) {
        fputs("empty table\n", stderr);
        return;
    }
}
