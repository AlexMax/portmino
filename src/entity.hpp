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

/**
 * Type of entity.
 */
typedef enum {
    MINO_ENTITY_NONE,
    MINO_ENTITY_RANDOM,
    MINO_ENTITY_PIECE,
    MINO_ENTITY_BOARD
} entity_type_t;

/**
 * Serialize function pointer.
 */
typedef buffer_t*(*entity_serialize_t)(void* ptr);

/**
 * Unserialize function pointer.
 */
typedef void*(*entity_unserialize_t)(buffer_t* ptr);

/**
 * Destructor function pointer.
 */
typedef void(*entity_destruct_t)(void* ptr);

/**
 * Configuration of entity.
 */
typedef struct entity_config_s {
    /**
     * Type of entity.
     */
    entity_type_t type;

    /**
     * Metatable used for userdata.
     */
    const char* metatable;

    /**
     * Entity serializer.
     */
    entity_serialize_t serialize;

    /**
     * Entity unserializer.
     */
    entity_unserialize_t unserialize;

    /**
     * Entity destructor.
     */
    entity_destruct_t destruct;
} entity_config_t;

/**
 * Generic entity.
 */
typedef struct entity_s {
    /**
     * Entity configuration.
     */
    entity_config_t config;

    /**
     * Entity unique id.
     */
    uint32_t id;

    /**
     * Registry reference for entity.
     */
    int registry_ref;

    /**
     * Opaque data member.
     */
    void* data;
} entity_t;

buffer_t* entity_serialize(entity_t* entity);
bool entity_unserialize(entity_t* entity, const buffer_t* buffer);
void entity_deinit(entity_t* entity);
