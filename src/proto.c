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

#include "proto.h"

#include <stdlib.h>

#include "error.h"

/**
 * A container that holds our loaded prototypes.
 */
typedef struct proto_container_s {
    /**
     * Prototypes array.
     */
    proto_t** data;

    /**
     * Number of prototypes in the container.
     */
    size_t size;

    /**
     * Currently allocated capacity of container.
     */
    size_t capacity;
} proto_container_t;

/**
 * Grow the prototype container
 */
static bool proto_container_grow(proto_container_t* protos) {
    proto_t** newdata = NULL;

    size_t newcap;
    if (protos->capacity == 0) {
        newcap = 16;
    } else {
        newcap = protos->capacity * 2;
    }
    newdata = reallocarray(protos->data, newcap, sizeof(*protos->data));

    if (newdata == NULL) {
        error_push_allocerr();
        return false;
    }

    protos->capacity = newcap;
    protos->data = newdata;

    return true;
}

/**
 * Allocate a container for holding prototypes.
 */
proto_container_t* proto_container_new(void) {
    proto_container_t* protos = NULL;

    if ((protos = calloc(1, sizeof(*protos))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    if (proto_container_grow(protos) == false) {
        // Error pushed by function
        goto fail;
    }

    return protos;

fail:
    proto_container_delete(protos);
    return NULL;
}

/**
 * Delete a prototype container.
 */
void proto_container_delete(proto_container_t* protos) {
    if (protos == NULL) {
        return;
    }

    if (protos->data != NULL) {
        // Free the contents of the data member
        for (size_t i = 0;i < protos->size;i++) {
            proto_delete(protos->data[i]);
            protos->data[i] = NULL;
        }

        // Free the data member
        free(protos->data);
        protos->data = NULL;
    }

    free(protos);
}

/**
 * Push a prototype into the container.
 *
 * The container takes ownership of the passed prototype.  You don't have to
 * worry about freeing it.
 */
bool proto_container_push(proto_container_t* protos, proto_t* proto) {
    if (protos->size >= protos->capacity) {
        if (proto_container_grow(protos) == false) {
            return false;
        }
    }

    protos->data[protos->size] = proto;
    protos->size += 1;

    return true;
}

/**
 * Allocate a new prototype
 */
proto_t* proto_new(proto_type_t type, void* data, proto_destruct_t destruct) {
    proto_t* proto = NULL;

    if ((proto = calloc(1, sizeof(*proto))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    proto->type = type;
    proto->data = data;
    proto->destruct = destruct;

    return proto;

fail:
    free(proto);
    return NULL;
}

/**
 * Delete the prototype
 */
void proto_delete(proto_t* proto) {
    if (proto == NULL) {
        return;
    }

    proto->destruct(proto->data);
    proto->type = MINO_PROTO_NONE;
    proto->destruct = NULL;
    proto->data = NULL;

    free(proto);
}
