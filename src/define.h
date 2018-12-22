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

/**
 * Target framerate of the gamestate.
 */
#define MINO_FPS 60

/**
 * Internal sample rate of all audio data.
 */
#define MINO_AUDIO_HZ 44100

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
