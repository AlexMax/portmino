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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#include "picture.h"

/**
 * Create a new picture from a file path.
 */
picture_t* picture_new(const char* path) {
    picture_t* pic = malloc(sizeof(picture_t));
    if (pic == NULL) {
        return NULL;
    }

    int x, y, bpp;
    pic->data = stbi_load(path, &x, &y, &bpp, 4);

    if (pic->data == NULL) {
        picture_delete(pic);
        return NULL;
    }

    if (x > UINT16_MAX || y > UINT16_MAX) {
        picture_delete(pic);
        return NULL;
    }

    pic->width = x;
    pic->height = y;

    // The library gives us the picture in RGBA format, but our renderers
    // expect ARGB, which is BGRA on little-endian machines.  So, swap the
    // red and blue channel.
    for (int i = 0;i < x * y * 4;i += 4) {
        uint8_t tmp = pic->data[i];
        pic->data[i] = pic->data[i+2];
        pic->data[i+2] = tmp;
    }

    return pic;
}

/**
 * Delete a picture structure.
 */
void picture_delete(picture_t* pic) {
    if (pic->data != NULL) {
        stbi_image_free(pic->data);
    }
    free(pic);
}
