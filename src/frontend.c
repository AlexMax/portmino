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

#include "frontend.h"

#include <string.h>

frontend_module_t g_frontend_module;

/**
 * Initialize any front-end functionality.
 */
bool frontend_init(const frontend_module_t* module) {
    memcpy(&g_frontend_module, module, sizeof(frontend_module_t));
    return true;
}

/**
 * Deinitialize the front-end.
 */
void frontend_deinit(void) {
    // Does nothing.
}

/**
 * Show an error message and terminate the game.
 */
void frontend_fatalerror(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    g_frontend_module.fatalerror(fmt, va);
    va_end(va);
}
