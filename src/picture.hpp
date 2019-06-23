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

picture_t* picture_new(int width, int height);
picture_t* picture_new_vfs(const char* path);
void picture_delete(picture_t* pic);
void picture_box(picture_t* dest, vec2i_t pos, vec2i_t len);
void picture_copy(picture_t* restrict dest, vec2i_t dstpos,
                  const picture_t* restrict source, vec2i_t srcpos);
void picture_blit(picture_t* restrict dest, vec2i_t dstpos,
                  const picture_t* restrict source, vec2i_t srcpos);
void picture_blit_alpha(picture_t* restrict dest, vec2i_t dstpos,
                        const picture_t* restrict source, vec2i_t srcpos,
                        uint8_t srcalpha);
