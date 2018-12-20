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

#define SOUND_SAMPLES 44100

typedef struct {
    /**
     * A pointer to the data to send to the audio subsystem.
     */
    int16_t* data;

    /**
     * The size of the data member in bytes.
     */
    size_t size;

    /**
     * The number of samples in the data.  Should be 44100Hz / 60 fps.
     */
    size_t samplecount;

    /**
     * The size of an individual sample.  Should be 16-bit stereo, so 32-bits.
     */
    size_t samplesize;
} audio_context_t;

void audio_init(void);
void audio_deinit(void);
audio_context_t* audio_frame(void);