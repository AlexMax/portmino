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

/**
 * Configuration variables for the playfield.
 */
typedef struct {
    /**
     * Width of the playfield.
     */
    int16_t width;

    /**
     * Complete height of the playfield.
     */
    int16_t height;

    /**
     * Visible height of the playfield, starting from the bottom.
     */
    int16_t visible_height;
} field_config_t;

/**
 * A single cell of the playfield is either 0 (empty) or contains a placed
 * block represented by a non-zero integer.
 */
typedef uint8_t field_cell_t;

typedef struct {
    /**
     * Size of the data member.  Usually the width * height of the playfield.
     */
    size_t size;

    /**
     * Playfield data of 'size' length.
     */
    field_cell_t* data;
} field_data_t;

typedef struct {
    /**
     * Configuration of playfield.
     */
    field_config_t config;

    /**
     * Contents of playfield.
     */
    field_data_t data;
} field_t;

field_t* field_new(void);
void field_delete(field_t* field);
