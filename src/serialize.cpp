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

#include "serialize.hpp"

#include "lua.hpp"
#include "mpack.h"

#include "entity.hpp"
#include "error.hpp"
#include "script.hpp"

static bool serialize_flat_value(lua_State* L, int flat, int index, mpack_writer_t* writer) {
    int top = lua_gettop(L);

    if (flat < 0) {
        // Translate into absolute index
        flat = top + flat + 1;
    }
    if (index < 0) {
        // Translate into absolute index
        index = top + index + 1;
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
    case LUA_TUSERDATA: {
        // Serialize a reference to the table or userdata.  Since things are
        // completely flat at this point, we can use an array for this.
        lua_pushvalue(L, index); // push table key to look up
        lua_rawget(L, flat); // pop table
        mpack_start_array(writer, 1);
        mpack_write_i64(writer, lua_tointeger(L, -1));
        mpack_finish_array(writer);
        lua_pop(L, 1);
        break;
    }
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    case LUA_TLIGHTUSERDATA:
        error_push("Data is not serializable.");
        return false;
    }

    return true;
}

static bool serialize_flat_index(lua_State* L, int flat, int index, mpack_writer_t* writer) {
    int top = lua_gettop(L);

    if (index < 0) {
        // First translate into absolute index
        index = top + index + 1;
    }

    // Flat data can either be userdata or table
    int type = lua_type(L, index);
    if (type == LUA_TUSERDATA) {
        entity_t* entity = NULL;
        buffer_t* data = NULL;

        // Assume userdata is an entity of some sort
        entity = lua_touserdata(L, index);
        if (entity == NULL) {
            error_push("Data is not serializable.");
            return false;
        }

        // Serialize the entity
        data = entity_serialize(entity);
        if (data == NULL) {
            error_push("Data is not serializable.");
            return false;
        }

        // Write serialized form as msgpack raw binary data
        uint8_t type = entity->config.type;
        mpack_start_bin(writer, 1 + data->size);
        mpack_write_bytes(writer, &type, sizeof(type));
        mpack_write_bytes(writer, (const char*)(data->data), data->size);
        mpack_finish_bin(writer);

        buffer_delete(data);
        return true;
    } else if (type != LUA_TTABLE) {
        error_push("Flat data is something other than a table or userdata.");
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
            // Dupe key to safely string-convert it.
            lua_pushvalue(L, -2); // dupe key
            size_t str_len = 0;
            const char* str = lua_tolstring(L, -1, &str_len);
            mpack_write_str(writer, str, str_len);
            lua_pop(L, 1); // pop dupe key
            if (serialize_flat_value(L, flat, -1, writer) == false) {
                // Function pushes error
                goto fail;
            }
            lua_pop(L, 1); // pop value
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
            if (serialize_flat_value(L, flat, -1, writer) == false) {
                // Function pushes error
                goto fail;
            }
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
 * Find all table and userdata references at a sepcific index and flatten
 * them into a single output table for ease of serialization.
 */
static bool flatten_table(lua_State* L, int index, int out, int* id) {
    int top = lua_gettop(L);

    if (index < 0) {
        // Translate into absolute index
        index = top + index + 1;
    }
    if (out < 0) {
        // Translate into absolute index
        out = top + out + 1;
    }

    if (lua_type(L, index) != LUA_TTABLE) {
        error_push("Tried to flatten a non-table.");
        return false;
    }

    // First check to see if the table is already a part of the output.
    lua_pushvalue(L, index);
    if (lua_rawget(L, out) != LUA_TNIL) {
        // Table already exists, we don't need to flatten it again.
        lua_settop(L, top);
        return true;
    }

    // Ensure that whatever we're pointing at is added to the output table
    lua_pushvalue(L, index);
    lua_pushinteger(L, *id);
    lua_rawset(L, out);

    // Future additions use the next id
    *id += 1;

    // Iterate the table we're currently pointing at looking for other tables
    // and userdata.
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        int type = lua_type(L, -1);
        switch (type) {
        case LUA_TTABLE:
            // Recursive call on the table
            flatten_table(L, -1, out, id);
            break;
        case LUA_TUSERDATA:
            // Add the userdata to the table directly.
            lua_pushinteger(L, *id);
            lua_rawset(L, out);
            *id += 1;

            // We already popped the value
            continue;
        default:
            // Do nothing.
            break;
        }

        lua_pop(L, 1); // pop value
    }

    lua_settop(L, top);
    return true;
}

static bool serialize_flat(lua_State* L, int index, mpack_writer_t* writer) {
    int top = lua_gettop(L);

    if (index < 0) {
        // Translate into absolute index
        index = top + index + 1;
    }

    // Push a serialization table.  This table keeps track of tables and
    // userdata that we've already serialized, and is discarded at the end.
    lua_newtable(L);
    int scratch = lua_gettop(L);

    // Flatten the table into an array of table and userdata references.
    int id = 1;
    if (!flatten_table(L, index, scratch, &id)) {
        // Error comes from function
        goto fail;
    }

    // Now that it's flat, serialize it.
    mpack_start_map(writer, id - 1);
    lua_pushnil(L);
    while (lua_next(L, scratch) != 0) {
        // Value contains the unique ID, key contains our table.
        mpack_write_int(writer, lua_tointeger(L, -1));
        if (serialize_flat_index(L, scratch, -2, writer) == false) {
            // Error comes from function
            goto fail;
        }
        lua_pop(L, 1);
    }
    mpack_finish_map(writer);

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
buffer_t* serialize_to_serialized(lua_State* L, int index) {
    buffer_t* msgpack = NULL;
    int top = lua_gettop(L);

    // Allocate our return buffer
    if ((msgpack = calloc(1, sizeof(buffer_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, (char**)(&msgpack->data), &msgpack->size);
    if (serialize_flat(L, index, &writer) == false) {
        error_push("Serialization error.");
        goto fail;
    }
    if (mpack_writer_destroy(&writer) != mpack_ok) {
        error_push("Serialization error.");
        goto fail;
    }

    lua_settop(L, top);
    return msgpack;

fail:
    lua_settop(L, top);
    buffer_delete(msgpack);
    return NULL;
}

static void unserialize_flat_value(lua_State* L, mpack_node_t* node) {
    int top = lua_gettop(L);
    mpack_type_t type = mpack_node_type(*node);

    // Unserialize depending on type
    switch (type) {
    case mpack_type_nil:
        lua_pushnil(L);
        break;
    case mpack_type_bool: {
        bool b = mpack_node_bool(*node);
        lua_pushboolean(L, (int)b);
        break;
    }
    case mpack_type_int: {
        int64_t i = mpack_node_i64(*node);
        lua_pushinteger(L, (lua_Integer)i);
        break;
    }
    case mpack_type_uint: {
        // Integers can exist as unsigned values in the message.  Since
        // we are the only writer of the message, assume it is in-bounds. 
        uint64_t i = mpack_node_u64(*node);
        lua_pushinteger(L, (lua_Integer)i);
        break;
    }
    case mpack_type_double: {
        double d = mpack_node_double(*node);
        lua_pushnumber(L, d);
        break;
    }
    case mpack_type_str: {
        // WARNING: str isn't null-terminated
        const char* str = mpack_node_str(*node);
        size_t len = mpack_node_strlen(*node);
        lua_pushlstring(L, str, len);
        break;
    }
    case mpack_type_array: {
        // This isn't actually an array, it's a stand-in value for an entity.
        // We can't resolve the entity at this juncture, so instead we store
        // the entity ID as the memory address of a light userdata.  It might
        // seem gross, but it's an integer value that doesn't allocate and
        // is distinct from a "real" number, so it's the best we've got.
        mpack_node_t entity = mpack_node_array_at(*node, 0);
        int64_t id = mpack_node_i64(entity);
        // FIXME: Pointers might be 32-bits.
        lua_pushlightuserdata(L, (void*)id);
        break;
    }
    default:
        fprintf(stderr, "default: %s\n", mpack_type_to_string(type));
        lua_pushnil(L);
        break;
    }
}

static void unflatten_table(lua_State* L, int flat, int index) {
    int top = lua_gettop(L);

    if (flat < 0) {
        // Translate into absolute index
        flat = top + flat + 1;
    }
    if (index < 0) {
        // Translate into absolute index
        index = top + index + 1;
    }

    // Fix up all instances of entity ID's so they point at their actual contents.
    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
        if (lua_type(L, -1) != LUA_TLIGHTUSERDATA) {
            // Not a table or a userdata, not interesting.
            lua_pop(L, 1);
            continue;
        }

        // Light userdata contains ID of original table or entity.
        lua_Integer id = (lua_Integer)lua_touserdata(L, -1);
        lua_pop(L, 1); // pop loop value
        lua_pushvalue(L, -1); // push loop key dupe
        lua_rawgeti(L, flat, id); // push flat entity
        if (lua_type(L, -1) == LUA_TTABLE) {
            unflatten_table(L, flat, -1); // Unflatten this subtable too
        }
        lua_rawset(L, index); // pop key and flat entity
    }
}

static bool unserialize_flat(lua_State* L, mpack_node_t* node) {
    int top = lua_gettop(L);

    // Despite being a map, the keys in the map start at 1 and end at the
    // item count - they're just in random order.
    size_t count = mpack_node_map_count(*node);

    // Push an output flat table to the top of the stack.
    lua_createtable(L, count, 0);

    // Loop over our keys.
    for (size_t i = 1;i <= count;i++) {
        mpack_node_t flatitem = mpack_node_map_uint(*node, i);
        mpack_type_t type = mpack_node_type(flatitem);
        switch (type) {
        case mpack_type_map: {
            // Normal Table
            size_t count = mpack_node_map_count(flatitem);
            lua_createtable(L, 0, count);
            for (size_t j = 0;j < count;j++) {
                mpack_node_t key = mpack_node_map_key_at(flatitem, j);
                unserialize_flat_value(L, &key); // push key
                mpack_node_t value = mpack_node_map_value_at(flatitem, j);
                unserialize_flat_value(L, &value); // push value
                lua_rawset(L, -3);
            }
            break;
        }
        case mpack_type_array: {
            // Table shaped like an array
            size_t count = mpack_node_array_length(flatitem);
            lua_createtable(L, count, 0);
            for (size_t j = 0;j < count;j++) {
                mpack_node_t value = mpack_node_array_at(flatitem, j);
                unserialize_flat_value(L, &value); // push value
                lua_rawseti(L, -2, j + 1);
            }
            break;
        }
        case mpack_type_bin: {
            // Userdata
            buffer_t buffer;
            buffer.data = (uint8_t*)mpack_node_bin_data(flatitem);
            buffer.size = mpack_node_bin_size(flatitem);

            // Unserialize the data into a userdata.
            entity_t* entity = lua_newuserdata(L, sizeof(entity_t));
            if (entity_unserialize(entity, &buffer) == false) {
                error_push("Could not unserialize entity.");
                goto fail;
            }

            // We use a different metatable for every possible entity type.
            luaL_setmetatable(L, entity->config.metatable);
            break;
        }
        default:
            error_push("Unexpected flat item type.");
            goto fail;
        }

        lua_rawseti(L, -2, i); // pop value
    }

    // Now we have a flattened table of tables and userdatas.  Start at the
    // first entry and walk recursively until all our flat entities are
    // resolved.
    lua_rawgeti(L, -1, 1); // push root table
    unflatten_table(L, -2, -1);

    return true;

fail:
    lua_settop(L, top);
    return false;
}

/**
 * Push a table to the stack with the contents of the given serialized buffer.
 */
void serialize_push_serialized(lua_State* L, const buffer_t* buffer) {
    mpack_tree_t tree;
    mpack_tree_init_data(&tree, buffer->data, buffer->size);
    mpack_tree_parse(&tree);

    mpack_node_t root = mpack_tree_root(&tree);
    if (unserialize_flat(L, &root) == false) {
        luaL_error(L, "Unserialization error.");
    }

    mpack_error_t error = mpack_tree_destroy(&tree);
    if (error != mpack_ok) {
        luaL_error(L, "Unserialization error.");
    }
}
