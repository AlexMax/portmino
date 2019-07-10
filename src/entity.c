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

#include "entity.h"

#include <stdlib.h>
#include <string.h>

#include "khash.h"
#include "mpack.h"

#include "error.h"
#include "serialize.h"

KHASH_MAP_INIT_INT64(entities, entity_t*);

/**
 * Entity manager instance
 */
typedef struct entity_manager_s {
    /**
     * Primary entity hashmap
     *
     * Keyed by a 64-bit ID, value is an allocated entity.
     */
    khash_t(entities)* entities;

    /**
     * Next available entity ID
     */
    uint64_t next_id;
} entity_manager_t;

/**
 * Serialize an entity
 */
buffer_t* entity_serialize(entity_t* entity) {
    buffer_t* buffer = NULL;
    mpack_writer_t writer;

    if ((buffer = calloc(1, sizeof(buffer_t))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Serialized data is an array that starts with the entity id and the
    // type id, followed by actual serialized data.
    mpack_writer_init_growable(&writer, (char**)(&buffer->data), &buffer->size);
    mpack_start_array(&writer, 3);
    mpack_write_u32(&writer, entity->id);
    mpack_write_u8(&writer, entity->config.type);
    entity->config.serialize(entity->data, &writer);
    mpack_finish_array(&writer);
    mpack_error_t err = mpack_writer_destroy(&writer);
    if (err != mpack_ok) {
        error_push("entity_serialize error: %s", mpack_error_to_string(err));
        goto fail;
    }

    return buffer;

fail:
    mpack_writer_destroy(&writer);
    buffer_delete(buffer);
    return NULL;
}

//
// FORWARD DECLARATIONS FOR UNSERIALIZE
//
// We need these here because including our various entities' header's would
// introduce a circular dependency.
//

typedef struct random_s random_t;
extern void random_entity_init(entity_t* entity);
extern random_t* random_unserialize(serialize_t* ser, mpack_reader_t* reader);
typedef struct piece_s piece_t;
extern void piece_entity_init(entity_t* entity);
extern piece_t* piece_unserialize(serialize_t* ser, mpack_reader_t* reader);
typedef struct board_s board_t;
extern void board_entity_init(entity_t* entity);
extern board_t* board_unserialize(serialize_t* ser, mpack_reader_t* reader);

/**
 * Unserialize to an entity
 * 
 * The first parameter should be an already-allocated entity that will have
 * its data overwritten by the new entity.  The resulting entity does not
 * have a registry reference set, that has to be supplied by the caller.
 */
bool entity_unserialize(entity_t* entity, serialize_t* ser, const buffer_t* buffer) {
    mpack_reader_t reader;
    mpack_reader_init_data(&reader, (const char*)buffer->data, buffer->size);

    // Serialized data is an array that starts with the entity id and the
    // type id, followed by actual serialized data.
    mpack_expect_array_match(&reader, 3);
    uint32_t id = mpack_expect_u32(&reader);
    uint8_t type = mpack_expect_u8(&reader);

    switch (type) {
    case MINO_ENTITY_RANDOM:
        random_entity_init(entity);
        entity->id = id;
        entity->data = random_unserialize(ser, &reader);
        break;
    case MINO_ENTITY_PIECE:
        piece_entity_init(entity);
        entity->id = id;
        entity->data = piece_unserialize(ser, &reader);
        break;
    case MINO_ENTITY_BOARD:
        board_entity_init(entity);
        entity->id = id;
        entity->data = board_unserialize(ser, &reader);
        break;
    default:
        error_push("Unknown entity ID (%u)", type);
        mpack_reader_destroy(&reader);
        return false;
    }

    entity->registry_ref = ser->registry_ref;

    mpack_done_array(&reader);

    mpack_error_t error = mpack_reader_destroy(&reader);
    if (error != mpack_ok) {
        error_push("MPack error (%s)", mpack_error_to_string(error));
        return false;
    }

    return true;
}

/**
 * Deinitialize an entity
 */
void entity_deinit(entity_t* entity) {
    if (entity == NULL) {
        return;
    }

    // If we have a destructor, use it
    if (entity->config.destruct != NULL) {
        entity->config.destruct(entity->data);
    }

    memset(entity, 0x00, sizeof(*entity));
}

/**
 * Allocate an entity manager
 */
entity_manager_t* entity_manager_new(void) {
    entity_manager_t* manager = NULL;
    khash_t(entities)* entities = NULL;

    manager = calloc(1, sizeof(*manager));
    if (manager == NULL) {
        error_push_allocerr();
        goto fail;
    }

    manager->entities = kh_init(entities);
    if (manager->entities == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // The first entity is id 1
    manager->next_id = 1;

    return manager;

fail:
    entity_manager_delete(manager);
    return NULL;
}

/**
 * Free the passed entity manager
 */
void entity_manager_delete(entity_manager_t* manager) {
    if (manager == NULL) {
        return;
    }

    // Delete all entities in the manager
    for (khint_t it = kh_begin(manager->entities);it != kh_end(manager->entities);it++) {
        if (kh_exist(manager->entities, it) == 1) {
            entity_t* entity = kh_val(manager->entities, it);
            entity_deinit(entity);
            free(entity);
            entity = NULL;
        }
    }

    kh_destroy(entities, manager->entities);
    manager->entities = NULL;

    free(manager);
}

/**
 * Create a new entity in the entity manager
 */
entity_t* entity_manager_create(entity_manager_t* manager) {
    entity_t* entity = NULL;

    entity = calloc(1, sizeof(*entity));
    if (entity == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Ensure that our entity is prepopulated with the proper id
    entity->id = manager->next_id;

    int ret;
    khint_t it = kh_put(entities, manager->entities, manager->next_id, &ret);
    if (ret == -1) {
        // Operation failed
        kh_del(entities, manager->entities, it);
        goto fail;
    } else if (ret == 0) {
        // Key exists - how did this happen?
        goto fail;
    }

    kh_value(manager->entities, it) = entity;

    manager->next_id += 1;
    return entity;

fail:
    entity_deinit(entity);
    free(entity);
    return NULL;
}

/**
 * Get an entity from the entity manager by entity id
 */
entity_t* entity_manager_get(entity_manager_t* manager, uint64_t id) {
    khint_t it = kh_get(entities, manager->entities, id);
    if (it == kh_end(manager->entities)) {
        // Key does not exist in hashtable
        return NULL;
    }

    return kh_val(manager->entities, it);
}

/**
 * Destroy an entity inside the entity manager by entity id
 */
void entity_manager_destroy(entity_manager_t* manager, uint64_t id) {
    // Find the entity
    entity_t* entity = entity_manager_get(manager, id);

    // Delete the entity - NULL entities are ignored
    entity_deinit(entity);
    free(entity);
    entity = NULL;

    // Delete the hashtable entry
    kh_del(entities, manager->entities, id);
}
