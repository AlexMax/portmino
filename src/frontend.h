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

#include <stdarg.h>

#include "define.h"

/**
 * Contains functionality that is specific to a frontend (libretro, SDL).
 */
typedef struct {
    /**
     * Call this when an unrecoverable error has occurred.
     */
    void (*fatalerror)(const char *fmt, va_list va);
} frontend_module_t;

bool frontend_init(const frontend_module_t* module);
void frontend_deinit(void);
void frontend_fatalerror(const char *fmt, ...);
