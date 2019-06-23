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

#include "entity.hpp"

#include <stdlib.h>
#include <string.h>

#include "error.hpp"

/**
 * Serialize an entity
 */
buffer_t* entity_serialize(entity_t* entity) {
    return entity->config.serialize(entity->data);
}

/**
 * Unserialize an entity
 */
bool entity_unserialize(entity_t* entity, const buffer_t* buffer) {
    // FIXME: entity doesn't exist yet, putting the cart before the horse here...
    DEBUG_BREAK;

    entity->data = entity->config.unserialize(buffer);
    if (entity->data == NULL) {
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

    entity->config.destruct(entity->data);
    memset(entity, 0x00, sizeof(*entity));
}
