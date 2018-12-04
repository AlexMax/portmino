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

picture_t* picture_new(void) {
    picture_t* pic = malloc(sizeof(picture_t));
    if (pic == NULL) {
        return NULL;
    }

    int x, y, bpp;
    pic->data = stbi_load("block.png", &x, &y, &bpp, 4);

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

    return pic;
}

void picture_delete(picture_t* pic) {
    if (pic->data == NULL) {
        stbi_image_free(pic->data);
    }
    free(pic);
}
