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

// Forward declarations.
typedef struct entity_s entity_t;

typedef struct random_s {
    /**
     * The current state of the random number generator.
     */
    uint32_t state[2];
} random_t;

random_t* random_new(uint32_t* seed);
void random_delete(random_t* random);
uint32_t random_number(random_t* random, uint32_t range);
buffer_t* random_serialize(random_t* random);
void random_entity_init(entity_t* entity);
