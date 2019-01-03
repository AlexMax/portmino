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

/**
 * Structure of a single hashmap entry.
 */
typedef struct {
    char* key;
    void* data;
} hmap_entry_t;

/**
 * Hashmap structure.
 */
typedef struct {
    hmap_entry_t* map;
    size_t count;
    size_t size;
} hmap_t;

hmap_t* hmap_new(void);
void hmap_delete(hmap_t* hmap);
void* hmap_find(hmap_t* hmap, const char* key);
void hmap_insert(hmap_t* hmap, const char* key, void* data);
