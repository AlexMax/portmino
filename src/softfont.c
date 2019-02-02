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

#include "softfont.h"

#include <stdlib.h>

#define MAX_GLYPHS 128

typedef struct softfont_s {
    picture_t* glyphs[MAX_GLYPHS];
} softfont_t;

/**
 * Allocate a software font from a filename.
 */
softfont_t* softfont_new(const char* path) {
    picture_t* fontpic = picture_new_vfs(path);
    if (fontpic == NULL) {
        return NULL;
    }

    softfont_t* font = calloc(1, sizeof(softfont_t));
    if (font == NULL) {
        picture_delete(fontpic);
        return NULL;
    }

    int glyph_width =  fontpic->width / 32;
    int glyph_height = fontpic->height / 4;

    // Software rendering requires individually sliced up font glyphs.
    for (size_t i = 0;i < MAX_GLYPHS;i++) {
        vec2i_t srcpos = {
            glyph_width * i % fontpic->width,
            glyph_height * i / fontpic->width * glyph_height
        };

        font->glyphs[i] = picture_new(glyph_width, glyph_height);
        if (font->glyphs[i] == NULL) {
            picture_delete(fontpic);
            softfont_delete(font);
            return NULL;
        }
        picture_copy(font->glyphs[i], vec2i_zero(), fontpic, srcpos);
    }

    picture_delete(fontpic);
    return font;
}

/**
 * Free a software font struct.
 */
void softfont_delete(softfont_t* font) {
    for (size_t i = 0;i < MAX_GLYPHS;i++) {
        if (font->glyphs[i] != NULL) {
            picture_delete(font->glyphs[i]);
            font->glyphs[i] = NULL;
        }
    }

    free(font);
}

/**
 * Render the given font into a destination picture.
 */
void softfont_render(softfont_t* font, picture_t* dst, vec2i_t dstpos, const char* text) {
    const unsigned char* c = (const unsigned char*)text;
    for (;*c != '\0';c++) {
        if (*c >= MAX_GLYPHS) {
            continue;
        }
        picture_blit(dst, dstpos, font->glyphs[*c], vec2i_zero());
        dstpos.x += font->glyphs[*c]->width;
    }
}
