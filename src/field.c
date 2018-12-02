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

#include <stdlib.h>

#include "field.h"

/**
 * Create a new playfield structure.
 * 
 * @return field_t* A newly allocated playfield.
 */
field_t* field_new(void) {
    field_t* field = malloc(sizeof(field_t));
    if (field == NULL) {
        abort();
    }

    // Define our configuration
    field->config.width = 10;
    field->config.height = 22;
    field->config.visible_height = 20;

    // Based on that configuration, construct the playfield itself
    size_t size = field->config.width * field->config.height;
    field->data.size = size;
    field->data.data = calloc(size, sizeof(field_cell_t));

    return field;
}

/**
 * Delete a playfield structure.
 * 
 * @param field The playfield structure to delete.
 */
void field_delete(field_t* field) {
    free(field->data.data);
    free(field);
}
