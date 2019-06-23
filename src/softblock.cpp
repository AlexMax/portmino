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

#include "softblock.hpp"

#include <stdlib.h>

#include "picture.hpp"

typedef struct softblock_s {
    picture_t** blocks;
    size_t count;
    size_t block_width;
    size_t block_height;
} softblock_t;

/**
 * Allocate a block atlas from a filename.
 */
softblock_t* softblock_new(const char* path) {
    picture_t* blockpic = NULL;
    softblock_t* block = NULL;

    blockpic = picture_new_vfs(path);
    if (blockpic == NULL) {
        goto fail;
    }

    block = calloc(1, sizeof(softblock_t));
    if (block == NULL) {
        goto fail;
    }

    // Blocks are assumed to be square, and our block count stems from that.
    block->block_width = blockpic->height;
    block->block_height = blockpic->height;
    block->count = blockpic->width / block->block_width;
    if (block->count == 0) {
        goto fail;
    }

    // Allocate our blocks.
    block->blocks = calloc(block->count, sizeof(picture_t*));
    if (block->blocks == NULL) {
        goto fail;
    }

    // Copy our block picture into the individual sliced up blocks.
    for (size_t i = 0;i < block->count;i++) {
        vec2i_t srcpos = {
            block->block_width * i, 0
        };

        block->blocks[i] = picture_new(block->block_width, block->block_height);
        if (block->blocks[i] == NULL) {
            goto fail;
        }
        picture_copy(block->blocks[i], vec2i_zero(), blockpic, srcpos);
    }

    picture_delete(blockpic);
    return block;

fail:
    picture_delete(blockpic);
    softblock_delete(block);
    return NULL;
}

/**
 * Free a block atlas.
 */
void softblock_delete(softblock_t* block) {
    if (block == NULL) {
        return;
    }

    if (block->blocks != NULL) {
        for (size_t i = 0;i < block->count;i++) {
            picture_delete(block->blocks[i]);
            block->blocks[i] = NULL;
        }
    }

    free(block->blocks);
    block->blocks = NULL;

    free(block);
}

/**
 * Get an individual block from the block atlas from ID.
 */
picture_t* softblock_get(const softblock_t* block, size_t index) {
    if (index >= block->count) {
        return NULL;
    }

    return block->blocks[index];
}
