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

#include "platform.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.hpp"
#include "vfs.hpp"

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
        g_config_dir = vfs_path_join(g_home_dir, ".config/" MINO_SUBDIR, '/');
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

    char* xdg_data_home = NULL;
    char* xdg_data_dirs = NULL;
    char* xdg_all_dirs = NULL;;

    // Home data directory
    xdg_data_home = getenv("XDG_DATA_HOME");
    if (xdg_data_home == NULL || xdg_data_home[0] == '\0') {
        // Not set.  Use the standard location.
        xdg_data_home = vfs_path_join(g_home_dir, ".local/share", '/');
        if (xdg_data_home == NULL) {
            error_push_allocerr();
            return NULL;
        }
    } else {
        // Duplicate the env, so we can free it later.
        xdg_data_home = strdup(xdg_data_home);
        if (xdg_data_home == NULL) {
            error_push_allocerr();
            return NULL;
        }
    }

    // XDG data directories
    xdg_data_dirs = getenv("XDG_DATA_DIRS");
    if (xdg_data_dirs == NULL) {
        // Not set.  Use the standard location.
        xdg_data_dirs = strdup("/usr/local/share/:/usr/share/");
        if (xdg_data_dirs == NULL) {
            error_push_allocerr();
            goto fail;
        }
    } else {
        // Duplicate the env, so we can free it later.
        xdg_data_dirs = strdup(xdg_data_dirs);
        if (xdg_data_dirs == NULL) {
            error_push_allocerr();
            goto fail;
        }
    }

    // Combine the list of directories into one big list.
    int ok = asprintf(&xdg_all_dirs, "%s:%s", xdg_data_home, xdg_data_dirs);
    free(xdg_data_home);
    xdg_data_home = NULL;
    free(xdg_data_dirs);
    xdg_data_dirs = NULL;
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    // Split the list of data dirs into separate strings.
    if ((g_data_dirs = calloc(1, sizeof(*g_data_dirs))) == NULL) {
        error_push_allocerr();
        goto fail;
    }
    *g_data_dirs = NULL; // Empty list

    size_t index = 0;
    char* token = NULL;
    char* next = xdg_all_dirs;
    while ((token = strtok_r(next, ":", &next)) != NULL) {
        // Join the current directory with the portmino subdirectory.
        char* dir = vfs_path_join(token, MINO_SUBDIR, '/');
        if (dir == NULL) {
            error_push_allocerr();
            goto fail;
        }

        // Make room for the next entry...
        char** new_data_dirs = reallocarray(g_data_dirs, (index + 2), sizeof(char*));
        if (new_data_dirs == NULL) {
            free(dir);
            error_push_allocerr();
            goto fail;
        }
        g_data_dirs = new_data_dirs;

        // Add the new directory to the list and increment our index...
        g_data_dirs[index++] = dir;

        // Our next index always begins life as NULL.  If this is our last
        // iteration, it will leave the list of data directories with a NULL
        // in the last position, ending it.
        g_data_dirs[index] = NULL;
    }

    free(xdg_all_dirs);
    return (const char**)g_data_dirs;

fail:
    free(xdg_data_home);
    free(xdg_data_dirs);
    free(xdg_all_dirs);

    if (g_data_dirs != NULL) {
        for (size_t i = 0;g_data_dirs[i] != NULL;i++) {
            free(g_data_dirs[i]);
            g_data_dirs[i] = NULL;
        }
    }
    free(g_data_dirs);
    g_data_dirs = NULL;

    return NULL;
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
    unix_random_get_seed
};
