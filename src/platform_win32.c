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

#include "platform.h"

static HCRYPTPROV g_crypt_provider;

static bool win32_init(void) {
    if (!CryptAcquireContext(&g_crypt_provider, NULL, NULL, PROV_RSA_FULL, 0)) {
        return false;
    }
    return true;
}

static void win32_deinit(void) {
    CryptReleaseContext(g_crypt_provider, 0);
}

static bool win32_random_get_seed(uint32_t* seed) {
    if (!CryptGenRandom(g_crypt_provider, sizeof(uint32_t), seed)) {
        return false;
    }
    return true;
}

platform_module_t g_platform_module = {
    win32_init,
    win32_deinit,
    win32_random_get_seed
};
