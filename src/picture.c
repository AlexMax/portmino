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

#include "picture.h"

#include <stdlib.h>
#include <string.h>

#include "stb_image.h"

#include "error.h"
#include "frontend.h"
#include "vfs.h"

/**
 * Size of a pixel of the native picture format, in bits.
 */
#define MINO_PICTURE_BPP 4

/**
 * Create a new empty picture.
 * 
 * Note that the data member is not initialized.  If you want an empty picture,
 * use picture_fill.
 */
picture_t* picture_new(int width, int height) {
    picture_t* pic = NULL;

    if (width > UINT16_MAX || height > UINT16_MAX) {
        return NULL;
    }

    if ((pic = calloc(1, sizeof(*pic))) == NULL) {
        error_push_allocerr();
        return NULL;
    }

    pic->width = width;
    pic->height = height;
    pic->size = width * height * MINO_PICTURE_BPP;

    if ((pic->data = malloc(pic->size)) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    return pic;

fail:
    free(pic);
    return NULL;
}

/**
 * Create a new picture from a virtual file path.
 */
picture_t* picture_new_vfs(const char* path) {
    vfile_t* file = NULL;
    picture_t* pic = NULL;

    if ((file = vfs_vfile_new(path)) == NULL) {
        error_push("Could not find picture %s.", path);
        goto fail;
    }

    if ((pic = calloc(1, sizeof(picture_t))) == NULL) {
        error_push_allocerr();
        return NULL;
    }

    int x, y, bpp;
    pic->data = stbi_load_from_memory(file->data, file->size,
        &x, &y, &bpp, MINO_PICTURE_BPP);
    if (pic->data == NULL) {
        error_push("Could not load picture %s", path);
        goto fail;
    }
    vfs_vfile_delete(file);
    file = NULL;

    if (x > UINT16_MAX || y > UINT16_MAX) {
        error_push("Picture (%s) dimensions are too large (%ux%u).", path, x, y);
        goto fail;
    }

    pic->width = x;
    pic->height = y;
    pic->size = x * y * MINO_PICTURE_BPP;

    // The library gives us the picture in RGBA format, but our renderers
    // expect ARGB, which is BGRA on little-endian machines.  So, swap the
    // red and blue channel.
    for (size_t i = 0;i < pic->size;i += MINO_PICTURE_BPP) {
        uint8_t tmp = pic->data[i];
        pic->data[i] = pic->data[i+2];
        pic->data[i+2] = tmp;
    }

    return pic;

fail:
    vfs_vfile_delete(file);
    picture_delete(pic);
    return NULL;
}

/**
 * Delete a picture structure.
 */
void picture_delete(picture_t* pic) {
    if (pic == NULL) {
        return;
    }

    free(pic->data); // stbi_image_free is just "free"
    free(pic);
}

/**
 * Copy one picture on top of another.  Mostly useful for software rendering.
 */
void picture_copy(picture_t* restrict dest, vec2i_t dstpos,
                  const picture_t* restrict source, vec2i_t srcpos) {
    // Where in the pixel data does our copying begin?
    int srccursor = (srcpos.y * source->width * MINO_PICTURE_BPP) + (srcpos.x * MINO_PICTURE_BPP);
    int dstcursor = (dstpos.y * dest->width * MINO_PICTURE_BPP) + (dstpos.x * MINO_PICTURE_BPP);

    // Our destination buffer might be smaller than the source.
    int copywidth, copyheight;
    if (source->width - srcpos.x > dest->width - dstpos.x) {
        copywidth = dest->width - dstpos.x;
    } else {
        copywidth = source->width - srcpos.x;
    }
    if (source->height - srcpos.y > dest->height - dstpos.y) {
        copyheight = dest->height - dstpos.y;
    } else {
        copyheight = source->height - srcpos.y;
    }

    for (int y = 0;y < copyheight;y++) {
        memcpy(dest->data + dstcursor, source->data + srccursor, copywidth * MINO_PICTURE_BPP);
        srccursor += source->width * MINO_PICTURE_BPP;
        dstcursor += dest->width * MINO_PICTURE_BPP;
    }
}

/**
 * Blit one picture on top of another, taking alpha into account.
 */
void picture_blit(picture_t* restrict dest, vec2i_t dstpos,
                  const picture_t* restrict source, vec2i_t srcpos) {
    // Where in the pixel data does our copying begin?
    int srccursor = (srcpos.y * source->width * MINO_PICTURE_BPP) + (srcpos.x * MINO_PICTURE_BPP);
    int dstcursor = (dstpos.y * dest->width * MINO_PICTURE_BPP) + (dstpos.x * MINO_PICTURE_BPP);

    // Our destination buffer might be smaller than the source.
    int copywidth, copyheight;
    if (source->width - srcpos.x > dest->width - dstpos.x) {
        copywidth = dest->width - dstpos.x;
    } else {
        copywidth = source->width - srcpos.x;
    }
    if (source->height - srcpos.y > dest->height - dstpos.y) {
        copyheight = dest->height - dstpos.y;
    } else {
        copyheight = source->height - srcpos.y;
    }

    for (int y = 0;y < copyheight;y++) {
        for (int xoff = 0;xoff < (copywidth * MINO_PICTURE_BPP);xoff += MINO_PICTURE_BPP) {
            // https://stackoverflow.com/a/12016968/91642
            //
            // This blends a source pixel into a destination pixel given
            // a specific alpha value.  This method uses only integers by
            // doing the calculation in 8.8 fixed point.
            uint_fast16_t alpha = source->data[srccursor + xoff + 3] + 1;
            uint_fast16_t inverse = 256 - source->data[srccursor + xoff + 3];
            uint8_t sb = source->data[srccursor + xoff];
            uint8_t sg = source->data[srccursor + xoff + 1];
            uint8_t sr = source->data[srccursor + xoff + 2];
            uint8_t db = dest->data[dstcursor + xoff];
            uint8_t dg = dest->data[dstcursor + xoff + 1];
            uint8_t dr = dest->data[dstcursor + xoff + 2];
            dest->data[dstcursor + xoff] = (alpha * sb + inverse * db) >> 8;
            dest->data[dstcursor + xoff + 1] = (alpha * sg + inverse * dg) >> 8;
            dest->data[dstcursor + xoff + 2] = (alpha * sr + inverse * dr) >> 8;
        }
        srccursor += source->width * MINO_PICTURE_BPP;
        dstcursor += dest->width * MINO_PICTURE_BPP;
    }
}
