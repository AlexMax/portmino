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

#include "piece.h"

/**
 * Type of prototype.
 */
typedef enum {
    PROTO_NONE,
    PROTO_PIECE,
    PROTO_BOARD
} proto_type_t;

/**
 * Destructor function pointer.
 * 
 * All prototype destructors must conform to this function type.
 */
typedef void (*proto_deinit_t)(void* ptr);

typedef struct proto_s {
    /**
     * Type of prototype.
     */
    proto_type_t type;

    /**
     * Opaque data member
     */
    void* data;

    /**
     * Prototype destructor
     */
    proto_deinit_t deinit;
} proto_t;

typedef struct proto_container_s proto_container_t;

proto_container_t* proto_container_new(void);
void proto_container_delete(proto_container_t* protos);
bool proto_container_push(proto_container_t* protos, proto_t* proto);
proto_t* proto_new(proto_type_t type, void* data, proto_deinit_t deinit);
void proto_delete(proto_t* proto);
