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

// There are some types and defines that we use all over the place, and it's
// just easier to include them all here.

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// We always have access to any compatibility defines.
#include "compat.h"

/**
 * Target framerate of the gamestate.
 */
#define MINO_FPS 60

/**
 * Internal sample rate of all audio data.
 */
#define MINO_AUDIO_HZ 44100

/**
 * Internal number of channels of all audio data.
 */
#define MINO_AUDIO_CHANNELS 2

/**
 * Annotate function as printf-style function.
 */
#ifdef __GNUC__
#define ATTRIB_PRINTF(index, first_arg) __attribute__ ((format(printf, index, first_arg)))
#else
#define ATTRIB_PRINTF(index, first_arg)
#endif

/**
 * Annotate function as unused.
 */
#ifdef __GNUC__
#define ATTRIB_UNUSED __attribute((unused))
#else
#define ATTRIB_UNUSED
#endif

/**
 * Count the number of items in a sized array.
 */
#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/**
 * Vector type of two integers
 */
typedef struct {
    int x;
    int y;
} vec2i_t;

/**
 * Return a vector with two values.
 */ 
static inline vec2i_t vec2i(int x, int y) {
    vec2i_t vec = { x, y };
    return vec;
}

/**
 * Return a vector of two integers initialized to (0, 0).
 */
static inline vec2i_t vec2i_zero(void) {
    vec2i_t zero = { 0, 0 };
    return zero;
}

/**
 * Generic buffer of bytes.
 */
typedef struct {
    uint8_t* data;
    size_t size;
} buffer_t;

void buffer_delete(buffer_t* buf);
