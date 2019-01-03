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

/**
 * This is a simple hashmap implementation.
 * 
 * Keys are strings, and items can only be added to the map, never removed.
 */

#include <stdlib.h>
#include <string.h>

#include "hmap.h"

/**
 * Create a new hashmap.
 */
hmap_t* hmap_new(void) {
    hmap_t* hmap = malloc(sizeof(hmap_t));

    // Allocate the map.
    hmap->size = 16;
    hmap->map = calloc(sizeof(hmap_entry_t), hmap->size);

    return hmap;
}

/**
 * Delete the hashmap.
 */
void hmap_delete(hmap_t* hmap) {
    for (size_t i = 0;i < hmap->size;i++) {
        if (hmap->map[i].key != NULL) {
            free(hmap->map[i].key);
        }
    }

    free(hmap->map);
    free(hmap);
}

/**
 * Calculate a unique 32-bit hash for a specific key.
 * 
 * Oh look, another implementation of djb2a.
 */
static uint32_t hmap_key_hash(const char* str) {
    uint32_t hash = 5381;
    unsigned char c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }

    return hash;
}

/**
 * Compare two keys to see if they're the same.
 */
static bool hmap_key_equal(const char* k1, const char* k2) {
    return strcmp(k1, k2) == 0 ? true : false;
}

/**
 * Find a hashmap entry given the raw map pointer and size.
 * 
 * This function is in theory capable of returning a NULL pointer if the
 * hashmap is completely full and the key cannot be found anywhere.  However,
 * the hashmap is supposed to grow when it becomes somewhat full, so this
 * should never happen.
 * 
 * This function exists so the calculation of which index to assign a key
 * is generic between our public API and our internal API.
 */
static hmap_entry_t* hmap_map_find(hmap_entry_t* map, size_t size, const char* key) {
    size_t index = hmap_key_hash(key) % size;

    for (size_t limit = 0;limit < size;limit++) {
        if (map[index].key == NULL) {
            return map + index;
        }
        if (hmap_key_equal(map[index].key, key)) {
            return map + index;
        }
        index = (index + 1) % size;
    }

    // Should never reach here.
    return NULL;
}

/**
 * Grow the size of the hashtable.
 */
static void hmap_grow(hmap_t* hmap) {
    size_t newsize = hmap->size * 2; // Grow by a power of two
    hmap_entry_t* newmap = calloc(sizeof(hmap_entry_t), hmap->size);

    // Rehash all of the entries in our hashmap.
    for (size_t i = 0;i < hmap->size;i++) {
        if (hmap->map[i].key == NULL) {
            // No need to rehash missing entries.
            continue;
        }

        // Find a new location for the old hashmap entry.
        hmap_entry_t* entry = hmap_map_find(newmap, newsize, hmap->map[i].key);
        entry->key = strdup(hmap->map[i].key);

        // Delete the old key.
        free(hmap->map[i].key);
    }

    // We're done with the old map.  Get rid of it.
    free(hmap->map);

    // We're left with only the new map data.
    hmap->size = newsize;
    hmap->map = newmap;
}

/**
 * Find an item in the hashmap.
 * 
 * Returns NULL if the item was not found.
 */
void* hmap_find(hmap_t* hmap, const char* key) {
    hmap_entry_t* entry = hmap_map_find(hmap->map, hmap->size, key);
    return entry->data;
}

/**
 * Insert an item into the hashmap.
 */
void hmap_insert(hmap_t* hmap, const char* key, void* data) {
    double full = hmap->count / (double)(hmap->size);
    if (full > 0.5) {
        hmap_grow(hmap);
    }

    hmap_entry_t* entry = hmap_map_find(hmap->map, hmap->size, key);
    entry->key = strdup(key);
    entry->data = data;
}
