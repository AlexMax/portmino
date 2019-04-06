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
     * Number of prototypes in the container.
     */
    size_t size;

    /**
     * Currently allocated capacity of container.
     */
    size_t capacity;

    /**
     * Prototypes array.
     */
    proto_t** data;
} proto_container_t;

/**
 * Grow the prototype container
 */
static bool proto_container_grow(proto_container_t* protos) {
    proto_t** newdata = NULL;
    
    size_t newcap;
    if (protos->capacity == 0) {
        newcap = 16;
        newdata = malloc(sizeof(*protos->data) * newcap);
    } else {
        newcap = protos->capacity * 2;
        newdata = realloc(protos, sizeof(*protos->data) * newcap);
    }

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
    (void)protos; 
    (void)proto;
    return true;
}
