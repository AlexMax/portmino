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

#include "frontend.h"
#include "picture.h"

/**
 * Size of a pixel of the native picture format, in bits.
 */
#define MINO_PICTURE_BPP 4

/**
 * Create a new picture from a virtual file path.
 */
picture_t* picture_new_vfs(const char* path) {
    picture_t* pic = malloc(sizeof(picture_t));
    if (pic == NULL) {
        return NULL;
    }
    memset(pic, 0, sizeof(picture_t));

    buffer_t* file = vfs_file(path);
    if (file == NULL) {
        picture_delete(pic);
        frontend_fatalerror("Could not find picture %s", path);
        return NULL;
    }

    int x, y, bpp;
    pic->data = stbi_load_from_memory(file->data, file->size,
        &x, &y, &bpp, MINO_PICTURE_BPP);
    if (pic->data == NULL) {
        picture_delete(pic);
        buffer_delete(file);
        frontend_fatalerror("Could not load picture %s", path);
        return NULL;
    }
    buffer_delete(file);

    if (x > UINT16_MAX || y > UINT16_MAX) {
        picture_delete(pic);
        return NULL;
    }

    pic->width = x;
    pic->height = y;
    pic->size = x * y * MINO_PICTURE_BPP;

    // The library gives us the picture in RGBA format, but our renderers
    // expect ARGB, which is BGRA on little-endian machines.  So, swap the
    // red and blue channel.
    for (int i = 0;i < pic->size;i += MINO_PICTURE_BPP) {
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
        free(pic->data); // stbi_image_free is just "free"
    }
    free(pic);
}

/**
 * Copy one picture on top of another.  Mostly useful for software rendering.
 * 
 * This function uses memcpy and is thus as fast as reasonably possible.
 * Transparency values are completely ignored.
 */
void picture_copy(picture_t* dest, const picture_t* source, int x, int y) {
    int cursor = (y * dest->width * MINO_PICTURE_BPP) + (x * MINO_PICTURE_BPP);
    int piccursor = 0;
    for (int i = 0;i < source->height;i++) {
        memcpy(dest->data + cursor, source->data + piccursor, source->width * MINO_PICTURE_BPP);
        cursor += dest->width * MINO_PICTURE_BPP;
        piccursor += source->width * MINO_PICTURE_BPP;
    }
}
