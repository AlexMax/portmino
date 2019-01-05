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

#include <stdio.h>
#include <stdlib.h>

#include "physfs.h"

#include "platform.h"
#include "vfs.h"

/**
 * Name of the default resource namespace
 */
#define MINO_DEFAULT_RESOURCE "basemino"

static bool vfs_load(const char* name) {
    char* pk3name = NULL;
    int ok = asprintf(&pk3name, "%s.pk3", name);
    if (ok < 0) {
        return false;
    }

    const char** data_dirs = platform()->data_dirs();
    for (size_t i = 0;data_dirs[i] != NULL;i++) {
        char* pk3dir = platform()->path_join(data_dirs[i], pk3name);
        if (pk3dir != NULL) {
            // Prioritize the pk3 first.
            PHYSFS_mount(pk3dir, NULL, 1);
            free(pk3dir);
        }

        char* dir = platform()->path_join(data_dirs[i], name);
        if (dir != NULL) {
            // Prioritize the dir of the same name second.
            PHYSFS_mount(dir, NULL, 1);
            free(dir);
        }
    }

    free(pk3name);
    return true;
}

/**
 * Initializes the virtual filesystem.
 */
bool vfs_init(void) {
    if (PHYSFS_init(NULL) == 0) {
        return false;
    }

    return vfs_load(MINO_DEFAULT_RESOURCE);
}

/**
 * Deinitialize the virtual filesystem.
 */
void vfs_deinit(void) {
    PHYSFS_deinit();
}

#include <stdio.h>

/**
 * Obtain file data by virtual filename.
 * 
 * The returned buffer is heap-allocated.  It is up to the caller to free it.
 */
buffer_t* vfs_file(const char* filename) {
    // Check in the VFS for the given file.
    PHYSFS_File* fh = PHYSFS_openRead(filename);
    if (fh == NULL) {
        PHYSFS_ErrorCode err = PHYSFS_getLastErrorCode();
        fprintf(stderr, "vfs_file(\"%s\"), %s\n", filename, PHYSFS_getErrorByCode(err));
        return NULL;
    }

    // Read the entire contents of the file into a buffer.
    buffer_t* filedata = malloc(sizeof(buffer_t));
    if (filedata == NULL) {
        PHYSFS_close(fh);
        return NULL;
    }

    filedata->size = PHYSFS_fileLength(fh);
    filedata->data = malloc(filedata->size);
    if (filedata->data == NULL) {
        PHYSFS_close(fh);
        free(filedata);
        return NULL;
    }

    PHYSFS_readBytes(fh, filedata->data, filedata->size);
    PHYSFS_close(fh);

    return filedata;
}
