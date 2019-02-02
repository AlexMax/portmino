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

#include "vfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physfs.h"

#include "platform.h"

/**
 * Name of the default resource namespace
 */
#define MINO_DEFAULT_RESOURCE "basemino"

/**
 * Designate loading paths for a resource pack with a given name.
 */
static bool vfs_load(const char* name) {
    char* pk3name = NULL;
    int ok = asprintf(&pk3name, "%s.pk3", name);
    if (ok < 0) {
        return false;
    }

    const char sep = PHYSFS_getDirSeparator()[0];
    const char** data_dirs = platform()->data_dirs();
    for (size_t i = 0;data_dirs[i] != NULL;i++) {
        char* pk3dir = vfs_path_join(data_dirs[i], pk3name, sep);
        if (pk3dir != NULL) {
            // Prioritize the pk3 first.
            PHYSFS_mount(pk3dir, NULL, 1);
            free(pk3dir);
        }

        char* dir = vfs_path_join(data_dirs[i], name, sep);
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

/**
 * Obtain file data by virtual filename.
 * 
 * The returned buffer must be freed by the caller.
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

/**
 * Join two filesystem paths together, ensuring there is a single directory
 * separator between them.
 * 
 * The returned string must be freed by the caller.
 * 
 * @param base Base path to append to.
 * @param append Path to append.
 * @param sep Directory separator.  PHYSFS paths always use '/'.
 * @return char* Joined path.
 */
char* vfs_path_join(const char* base, const char* append, const char sep) {
    if (strlen(base) == 0) {
        // There is no base to append to.
        return NULL;
    }

    if (strlen(append) == 0) {
        // Return a duplicated version of the base path.
        return strdup(base);
    }

    int bytes = 0;
    char* result = NULL;
    if (base[strlen(base) - 1] == sep) {
        if (append[0] == sep) {
            // Base and append have slashes.  Truncate one of the slashes.
            char* a_base = strdup(base);
            if (a_base == NULL) {
                return NULL;
            }
            a_base[strlen(a_base) - 1] = '\0';
            bytes = asprintf(&result, "%s%c%s", a_base, sep, append);
            free(a_base);
        } else {
            // Base has a slash, append is missing one.
            bytes = asprintf(&result, "%s%s", base, append);
        }
    } else {
        if (append[0] == sep) {
            // Base does not have a slash, append has one.
            bytes = asprintf(&result, "%s%s", base, append);
        } else {
            // Base and append do not have slashes, insert one.
            bytes = asprintf(&result, "%s%c%s", base, sep, append);
        }
    }

    if (bytes < 0) {
        // Did not allocate
        return NULL;
    }

    // Path should be joined now...
    return result;
}
