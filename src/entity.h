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

#include "define.h"

/**
 * Type of entity.
 */
typedef enum {
    MINO_ENTITY_NONE,
    MINO_ENTITY_PIECE,
    MINO_ENTITY_BOARD
} entity_type_t;

/**
 * Destructor function pointer.
 *
 * All prototype destructors must conform to this function type.
 */
typedef void(*entity_destruct_t)(void* ptr);

/**
 * Generic entity.
 */
typedef struct entity_s {
    /**
     * Entity unique id.
     */
    uint32_t id;

    /**
     * Registry reference for entity.
     */
    int registry_ref;

    /**
     * Type of entity.
     */
    entity_type_t type;

    /**
     * Opaque data member.
     */
    void* data;

    /**
     * Entity destructor.
     */
    entity_destruct_t destruct;
} entity_t;

void entity_deinit(entity_t* entity);
