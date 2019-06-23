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

extern platform_module_t g_platform_module;

/**
 * Initialize any platform-specific functionality.
 */
bool platform_init(void) {
    return g_platform_module.init();
}

/**
 * Cleanup any platform-specific functionality.
 */
void platform_deinit(void) {
   g_platform_module.deinit();
}

/**
 * Return a pointer to the current platform module.
 */
platform_module_t* platform(void) {
    return &g_platform_module;
}
