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

#include "platform.h"

FILE* g_random;

static bool unix_init(void) {
    g_random = fopen("/dev/urandom", "rb");
    if (g_random == NULL) {
        return false;
    }

    return true;
}

static void unix_deinit(void) {
    if (g_random != NULL) {
        fclose(g_random);
        g_random = NULL;
    }
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
    unix_random_get_seed
};
