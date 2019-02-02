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

#include "define.h"

// Forward declarations
typedef struct sound_s sound_t;

typedef struct audio_context_s {
    /**
     * A pointer to the data to send to the audio subsystem.
     */
    int16_t* sampledata;

    /**
     * The size of the data member in bytes.
     */
    size_t bytesize;

    /**
     * The number of frames in the data.  Should be 44100Hz / 60 fps.
     */
    size_t framecount;

    /**
     * The size of an individual frame.  Should be 16-bit stereo, so 32-bits.
     */
    size_t sizeofframe;
} audio_context_t;

extern sound_t* g_sound_gameover;
extern sound_t* g_sound_lock;
extern sound_t* g_sound_move;
extern sound_t* g_sound_piece0;
extern sound_t* g_sound_rotate;
extern sound_t* g_sound_step;

void audio_init(void);
void audio_deinit(void);
void audio_playsound(const sound_t* sound);
audio_context_t* audio_frame(size_t samples);
