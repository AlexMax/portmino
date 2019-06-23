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

#include "platform.h"

#include <stdlib.h>
#include <time.h>

static bool emscripten_init(void) {
    srand(time(NULL));
    return true;
}

static void emscripten_deinit(void) {
    // Do nothing
}

static const char* emscripten_config_dir(void) {
    return "/etc/portmino/";
}

static const char** emscripten_data_dirs(void) {
    static const char* data_dirs[] = {
        "/usr/share/portmino/",
        NULL
    };
    return data_dirs;
}

static bool emscripten_random_get_seed(uint32_t* seed) {
    // Emscripten seems to have problems with /dev/urandom
    *seed = rand();
    return true;
}

platform_module_t g_platform_module = {
    emscripten_init,
    emscripten_deinit,
    emscripten_config_dir,
    emscripten_data_dirs,
    emscripten_random_get_seed
};
