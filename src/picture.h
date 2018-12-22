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
     * Picture data, BGRA format.
     */
    uint8_t* data;

    /**
     * Size of picture data.
     */
    size_t size;

    /**
     * Width of the picture.
     */
    uint16_t width;

    /**
     * Height of the picture.
     */
    uint16_t height;
} picture_t;

picture_t* picture_new(const char* path);
void picture_delete(picture_t* pic);
void picture_copy(picture_t* dest, const picture_t* source, int x, int y);
