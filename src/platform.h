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

#pragma once

#include "define.h"

typedef struct {
    /**
     * Platform-specific init.
     */
    bool (*init)(void);

    /**
     * Platform-specific cleanup.
     */
    void (*deinit)(void);

    /**
     * Get a 32-bit random seed for the random number generator.
     */
    bool (*random_get_seed)(uint32_t* seed);
} platform_module_t;

bool platform_init(void);
void platform_deinit(void);
platform_module_t* platform(void);
