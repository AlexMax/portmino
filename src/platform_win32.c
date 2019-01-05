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

#include <Windows.h>
#include <Wincrypt.h>

#include "physfs.h"

#include "platform.h"

static HCRYPTPROV g_crypt_provider;

/**
 * Array of data directories.  Ends with a NULL.
 */
static const char** g_data_dirs;

static bool win32_init(void) {
    if (!CryptAcquireContext(&g_crypt_provider, NULL, NULL, PROV_RSA_FULL, 0)) {
        return false;
    }
    return true;
}

static void win32_deinit(void) {
    CryptReleaseContext(g_crypt_provider, 0);

    if (g_data_dirs != NULL) {
        free(g_data_dirs);
        g_data_dirs = NULL;
    }
}

static const char* win32_config_dir(void) {
    return NULL;
}

/**
 * Windows only has one correct place to grab data from - the directory of
 * the executable.
 */
static const char** win32_data_dirs(void) {
    if (g_data_dirs != NULL) {
        return g_data_dirs;
    }

    g_data_dirs = malloc(sizeof(const char*) * 2);
    if (g_data_dirs == NULL) {
        return NULL;
    }

    // Why use Win32 when PHYSFS already gives us what we want?
    g_data_dirs[0] = PHYSFS_getBaseDir();
    g_data_dirs[1] = NULL;

    return g_data_dirs;
}

static bool win32_random_get_seed(uint32_t* seed) {
    if (!CryptGenRandom(g_crypt_provider, sizeof(uint32_t), (BYTE*)seed)) {
        return false;
    }
    return true;
}

platform_module_t g_platform_module = {
    win32_init,
    win32_deinit,
    win32_config_dir,
    win32_data_dirs,
    win32_random_get_seed
};
