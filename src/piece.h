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

#include <stddef.h>
#include <stdint.h>

typedef struct {
    /**
     * What a piece looks like from every rotation.
     * 
     * Total size of this pointer is data_size * data_count.
     */
    uint8_t* datas;

    /**
     * Total width of a single piece, including blank space.
     */
    uint8_t width;

    /**
     * Total height of a single piece, including blank space.
     */
    uint8_t height;

    /**
     * The individual size of each data entry in bytes.
     */
    size_t data_size;

    /**
     * How many rotations a piece has.  Usually 4.
     */
    uint8_t data_count;
} piece_config_t;

typedef struct {
    /**
     * X coordinate of the current piece (left side).  Can be off the board.
     */
    int16_t x;

    /**
     * Y coordinate of the current piece (top side).  Can be off the board.
     */
    int16_t y;

    /**
     * Current orientation of the piece.
     */
    uint8_t rot;

    /**
     * Configuration of the piece.
     * 
     * The piece does not own this pointer, so don't malloc or free it.
     */
    piece_config_t* config;
} piece_t;

piece_t* piece_new_s(void);
void piece_delete(piece_t* piece);
