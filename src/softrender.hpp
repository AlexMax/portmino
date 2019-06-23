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

#include "define.hpp"
#include "picture.hpp"
#include "render.hpp"

#define MINO_SOFTRENDER_WIDTH 320
#define MINO_SOFTRENDER_HEIGHT 240
#define MINO_SOFTRENDER_BPP 4

typedef struct {
    /**
     * The actual buffer of pixels to draw to.  It's in ARGB format, which
     * on little-endian machines is actually BGRA.
     */
    picture_t buffer;
} softrender_context_t;

extern render_module_t softrender_module;
