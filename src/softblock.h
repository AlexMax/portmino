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

#include "picture.h"

typedef struct {
    picture_t** blocks;
    size_t count;
    size_t block_width;
    size_t block_height;
} softblock_t;

softblock_t* softblock_new(const char* path);
void softblock_delete(softblock_t* block);
picture_t* softblock_get(const softblock_t* block, size_t index);
