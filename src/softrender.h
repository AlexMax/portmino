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

#include <stdint.h>

#include "render.h"

typedef struct {
    /**
     * Our render buffer which contains the actual picture to be rendered.
     * 
     * Each pixel is stored as ARGB, technically BGRA on little-endian machines.
     */
    uint8_t* buffer;

    /**
     * Size of the render buffer in bytes.
     */
    size_t size;

    /**
     * Width of the render buffer in bytes.
     */
    uint16_t width;

    /**
     * Height of the render buffer in bytes.
     */
    uint16_t height;
} softrender_context_t;

extern render_module_t soft_render_module;
