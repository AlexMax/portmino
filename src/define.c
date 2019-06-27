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

#include "define.h"

#include <stdlib.h>
#include <stdio.h>

void buffer_delete(buffer_t* buf) {
    if (buf == NULL) {
        return;
    }

    if (buf->data != NULL) {
        free(buf->data);
    }

    free(buf);
}

void buffer_debug(const buffer_t* buf) {
    fprintf(stderr, "ptr: %p, size: %zu\ndata: ", buf, buf->size);
    for (size_t i = 0;i < buf->size;i++) {
        fprintf(stderr, "%02X ", buf->data[i]);
    }
    fprintf(stderr, "\n");
}
