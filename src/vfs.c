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
 * This file contains a virtual filesystem abstraction that stores game
 * resources.
 * 
 * Our program expects to have access to certain files that contain the
 * resources necessary to play the game.  This includes things such as graphics
 * and sound.  There are a number of places where these resources could be
 * found, and there needs to be one unified method of accessing them.
 * 
 * - Base resources are either embedded in the binary or on-disk in the
 *   program directory.
 * - Addon resources are loaded from a configuration directory.
 * 
 * This abstraction is not concerned with writable files such as configurations
 * or high score tables.  Use a generic filesystem abstraction for that.
 */

#include <stdio.h>
#include <stdlib.h>

#include "miniz.h"

#include "platform.h"
#include "vfs.h"

/**
 * Attempt to locate and load the contents of a filesystem into the virtual
 * filesystem.
 * 
 * The "name" parameter refers to the means of uniquely identifying the
 * filesystem to load into the VFS.  It is either the name of the directory
 * or the name of the pk3 file on disk.  Passing the name "base" will also
 * check, if it exists in this build, whatever embedded filesystem we have
 * access to.
 */
static bool vfs_load(vfs_t* vfs, const char* name) {
    // Where are the data directories located?
    const char** data_dirs = platform()->data_dirs();

    // TODO: First we load the contents of our built-in resource, if they exist.

    // Add pk3 to the name to get the filename
    char* filename = NULL;
    int bytes = asprintf(&filename, "%s.pk3", name);
    if (bytes < 0) {
        free(data_dirs);
        return false;
    }

    // Next we load any packaged file we find on disk.
    for (size_t i = 0;data_dirs[i] != NULL;i++) {
        char* fullname = platform()->path_join(data_dirs[i], filename);
        buffer_t filedata = { 0 };
        if (platform()->file_get_contents(fullname, &filedata)) {
            fprintf(stderr, "Loaded: [%s]\n", fullname);
            mz_zip_archive zip_archive;
            mz_zip_zero_struct(&zip_archive);
            mz_bool ok = mz_zip_reader_init_mem(&zip_archive, filedata.data, filedata.size, 0);
            // int file_index = mz_zip_reader_locate_file(&zip_archive, filename, NULL, 0);
            // if (file_index > 0) {
            //     mz_bool is_dir = mz_zip_reader_is_file_a_directory(&zip_archive, file_index);
            //     filedata.data = mz_zip_reader_extract_to_heap(&zip_archive, file_index, &filedata.size, 0);
            // }
            mz_zip_reader_end(&zip_archive);
        }
        free(fullname);
    }

    // Finally, we load all files we find in any directory with the passed
    // name we find on disk.
    for (size_t i = 0;data_dirs[i] != NULL;i++) {
        char* dirname = platform()->path_join(data_dirs[i], name);
        fprintf(stderr, "[%s]\n", dirname);
        free(dirname);
    }

    return true;
}

/**
 * Initializes the virtual filesystem.
 */
vfs_t* vfs_new(void) {
    vfs_t* vfs = malloc(sizeof(vfs_t));
    if (vfs == NULL) {
        return NULL;
    }

    vfs->files = hmap_new();

    // By default, load the base configuration.
    vfs_load(vfs, "base");

    return vfs;
}

/**
 * Deinitialize the virtual filesystem.
 */
void vfs_delete(vfs_t* vfs) {
    if (vfs->files != NULL) {
        hmap_delete(vfs->files);
        vfs->files = NULL;
    }

    free(vfs);
}

/**
 * Obtain file data by virtual filename.
 * 
 * If there is an error retriving file data, the returned pointer will be NULL.
 */
buffer_t* vfs_file(const vfs_t* vfs, const char* filename) {
    // Is it sitting on disk?
    buffer_t* filedata = malloc(sizeof(buffer_t));
    if (platform()->file_get_contents(filename, filedata)) {
        hmap_insert(vfs->files, filename, filedata);
        return filedata;
    }

    // We can't find the data anywhere.
    free(filedata);
    return NULL;
}
