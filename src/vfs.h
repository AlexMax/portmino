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

typedef enum {
    /**
     * Don't push an error on failure.
     */
    MINO_VFILE_NOERR = 1 << 0,
} vfile_flag_t;

typedef unsigned vfile_flags_t;

/**
 * A file obtained from the virtual filesystem
 */
typedef struct vfile_s {
    /**
     * The original filename of the file.
     */
    char* filename;

    /**
     * Raw binary data of the file.
     */
    uint8_t* data;

    /**
     * Size of the data member in bytes.
     */
    size_t size;
} vfile_t;

bool vfs_init(const char* argv0);
void vfs_deinit(void);
vfile_t* vfs_vfile_new(const char* filename, vfile_flags_t flags);
void vfs_vfile_delete(vfile_t* file);
char* vfs_path_join(const char* base, const char* append, char sep);
