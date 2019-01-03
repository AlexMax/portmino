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
#include "hmap.h"

typedef struct {
    /**
     * A cache of recently-loaded files and their data.
     */
    hmap_t* files;
} vfs_t;

vfs_t* vfs_new(void);
void vfs_delete(vfs_t* vfs);
buffer_t* vfs_file(const vfs_t* vfs, const char* filename);
