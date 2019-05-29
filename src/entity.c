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

#include "error.h"

/**
 * Serialize an entity
 */
buffer_t* entity_serialize(entity_t* entity) {
    return entity->serialize(entity->data);
}

/**
 * Deinitialize an entity
 */
void entity_deinit(entity_t* entity) {
    if (entity == NULL) {
        return;
    }

    entity->destruct(entity->data);
    entity->type = MINO_ENTITY_NONE;
    entity->destruct = NULL;
    entity->data = NULL;
}
