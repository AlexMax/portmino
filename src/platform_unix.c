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
#include <string.h>

#include "platform.h"

/**
 * The subdirectory to use with all of our directories.
 */
#define MINO_SUBDIR "portmino"

/**
 * Random number generator file handle.
 */
static FILE* g_random;

/**
 * Configuration directory.
 */
static char* g_config_dir;

/**
 * Array of data directories.  Ends with a NULL.
 */
static char** g_data_dirs;

/**
 * Home directory.
 */
static char* g_home_dir;

// Platform functions we can call from other functions - saves on an indirection.
static char* unix_path_join(const char* base, const char* append);

static bool unix_init(void) {
    g_random = fopen("/dev/urandom", "rb");
    if (g_random == NULL) {
        return false;
    }

    char* home_dir = getenv("HOME");
    if (home_dir == NULL) {
        fclose(g_random);
        return false;
    }

    g_home_dir = strdup(home_dir);
    if (g_home_dir == NULL) {
        fclose(g_random);
        return false;
    }

    return true;
}

static void unix_deinit(void) {
    if (g_random != NULL) {
        fclose(g_random);
        g_random = NULL;
    }

    if (g_config_dir != NULL) {
        free(g_config_dir);
        g_config_dir = NULL;
    }

    if (g_home_dir != NULL) {
        free(g_home_dir);
        g_home_dir = NULL;
    }

    if (g_data_dirs != NULL) {
        for (size_t i = 0;g_data_dirs[i] != NULL;i++) {
            free(g_data_dirs[i]);
            g_data_dirs[i] = NULL;
        }

        free(g_data_dirs);
        g_data_dirs = NULL;
    }
}

/**
 * The location of configuration directories on *NIX depends on the XDG base
 * directory standard.
 */
static const char* unix_config_dir(void) {
    if (g_config_dir != NULL) {
        return g_config_dir;
    }

    // Configuration directory
    char* xdg_config_home = getenv("XDG_CONFIG_HOME");
    if (xdg_config_home != NULL) {
        g_config_dir = strdup(xdg_config_home);
    } else {
        g_config_dir = unix_path_join(g_home_dir, ".config/" MINO_SUBDIR);
    }

    return g_config_dir;
}

/**
 * The location of data directories on *NIX depends on the XDG base directory
 * standard.
 */
static const char** unix_data_dirs(void) {
    if (g_data_dirs != NULL) {
        return (const char**)g_data_dirs;
    }

    // Home data directory
    char* xdg_data_home = getenv("XDG_DATA_HOME");
    if (xdg_data_home == NULL) {
        xdg_data_home = unix_path_join(g_home_dir, ".local/share");
        if (xdg_data_home == NULL) {
            return NULL;
        }
    } else {
        xdg_data_home = strdup(xdg_data_home);
        if (xdg_data_home == NULL) {
            return NULL;
        }
    }

    // XDG data directories
    char* xdg_data_dirs = getenv("XDG_DATA_DIRS");
    if (xdg_data_dirs == NULL) {
        xdg_data_dirs = strdup("/usr/local/share/:/usr/share/");
        if (xdg_data_dirs == NULL) {
            free(xdg_data_home);
            return NULL;
        }
    } else {
        xdg_data_dirs = strdup(xdg_data_dirs);
        if (xdg_data_dirs == NULL) {
            free(xdg_data_home);
            return NULL;
        }
    }

    // Combine the list of directories into one big list.
    char* xdg_all_dirs;
    int bytes = asprintf(&xdg_all_dirs, "%s:%s", xdg_data_home, xdg_data_dirs);
    free(xdg_data_home);
    free(xdg_data_dirs);
    if (bytes < 0) {
        // Allocation was unsuccessful.
        return NULL;
    }

    // Split the list of data dirs into separate strings.
    g_data_dirs = malloc(sizeof(char*));
    if (g_data_dirs == NULL) {
        // Allocation was unsuccessful.
        return NULL;
    }

    size_t index = 0;
    char* token = NULL;
    char* next = xdg_all_dirs;
    while ((token = strtok_r(next, ":", &next)) != NULL) {
        // Join the current directory with the portmino subdirectory.
        char* dir = unix_path_join(token, MINO_SUBDIR);
        if (dir == NULL) {
            break;
        }

        // Make room for the next entry...
        char** new_data_dirs = realloc(g_data_dirs, sizeof(char*) * (index + 2));
        if (new_data_dirs == NULL) {
            break;
        }
        g_data_dirs = new_data_dirs;

        // Add the new directory to the list and increment our index...
        g_data_dirs[index++] = dir;

        // Our next index always begins life as NULL.  If this is our last
        // iteration, it will leave the list of data directories with a NULL
        // in the last position, ending it.
        g_data_dirs[index] = NULL;
    }

    // If our iteration stopped midway through, ensure that the last index
    // we were looking at is set to NULL.
    if (g_data_dirs[index] != NULL) {
        free(g_data_dirs[index]);
        g_data_dirs[index] = NULL;
    }

    free(xdg_all_dirs);
    return (const char**)g_data_dirs;
}

static bool unix_file_get_contents(const char* filename, buffer_t* buffer) {
    // Open the file.
    FILE* fh = fopen(filename, "r");
    if (fh == NULL) {
        return false;
    }

    // Get the length of the file.
    fseek(fh, 0, SEEK_END);
    buffer->size = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    buffer->data = malloc(buffer->size);
    if (buffer->data == NULL) {
        fclose(fh);
        return false;
    }

    fread(buffer->data, buffer->size, 1, fh);
    fclose(fh);

    return true;
}

static char* unix_path_join(const char* base, const char* append) {
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
    if (base[strlen(base) - 1] == '/') {
        if (append[0] == '/') {
            // Base and append have slashes.  Truncate one of the slashes.
            char* a_base = strdup(base);
            if (a_base == NULL) {
                return NULL;
            }
            a_base[strlen(a_base) - 1] = '\0';
            bytes = asprintf(&result, "%s/%s", a_base, append);
            free(a_base);
        } else {
            // Base has a slash, append is missing one.
            bytes = asprintf(&result, "%s%s", base, append);
        }
    } else {
        if (append[0] == '/') {
            // Base does not have a slash, append has one.
            bytes = asprintf(&result, "%s%s", base, append);
        } else {
            // Base and append do not have slashes, insert one.
            bytes = asprintf(&result, "%s/%s", base, append);
        }
    }

    if (bytes < 0) {
        // Did not allocate
        return NULL;
    }

    // Path should be joined now...
    return result;
}

static bool unix_random_get_seed(uint32_t* seed) {
    size_t items = fread(seed, sizeof(uint32_t), 1, g_random);
    if (items < 1) {
        return false;
    }

    return true;
}

platform_module_t g_platform_module = {
    unix_init,
    unix_deinit,
    unix_config_dir,
    unix_data_dirs,
    unix_file_get_contents,
    unix_path_join,
    unix_random_get_seed
};
