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

#include "ruleset.h"

/**
 * Allocate a new next piece buffer.
 */
next_t* next_new(ruleset_t* ruleset, size_t id) {
    next_t* next = calloc(1, sizeof(next_t));
    if (next == NULL) {
        return NULL;
    }

    next->id = id;
    next->next_index = 0;

    for (size_t i = 0;i < MAX_NEXTS;i++) {
        next->nexts[i] = ruleset_next_piece(ruleset, next);
        if (next->nexts[i] == NULL) {
            next_delete(next);
            return NULL;
        }
    }

    return next;
}

/**
 * Delete the next piece buffer.
 */
void next_delete(next_t* next) {
    for (size_t i = 0;i < MAX_NEXTS;i++) {
        next->nexts[i] = NULL;
    }
    free(next);
}

/**
 * Return the configuration of the next piece
 */
const piece_config_t* next_get_next_piece(const next_t* next, size_t index) {
    index = next->next_index + index % MAX_NEXTS;
    return next->nexts[index];
}

/**
 * "Consume" the next piece
 * 
 * Advances the next-piece index, wrapping around if necessary, and generates
 * a new next piece at the end.
 */
void next_consume_next_piece(next_t* next, ruleset_t* ruleset) {
    next->nexts[next->next_index] = ruleset_next_piece(ruleset, next);
    next->next_index = (next->next_index + 1) % MAX_NEXTS;
}
