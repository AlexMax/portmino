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

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "sound.h"

#define MINO_AUDIO_CHANNELS 2

static audio_context_t g_audio_ctx;

static sound_t* g_sound_piece0;

/**
 * Initialize the audio context.
 */
void audio_init(void) {
    g_audio_ctx.samplecount = MINO_AUDIO_HZ / MINO_FPS;
    g_audio_ctx.samplesize = sizeof(int16_t) * MINO_AUDIO_CHANNELS;
    g_audio_ctx.size = g_audio_ctx.samplecount * g_audio_ctx.samplesize;
    g_audio_ctx.data = malloc(g_audio_ctx.size);

    g_sound_piece0 = sound_new("../res/sfx/piece0.wav");
}

/**
 * Clean up the audio context.
 */
void audio_deinit(void) {
    if (g_audio_ctx.data != NULL) {
        free(g_audio_ctx.data);
        g_audio_ctx.data = NULL;
    }
}

static size_t start;
static size_t end;

/**
 * Set up a game-frame's worth of audio data and pass it back to whatever
 * is playing our audio.
 */
audio_context_t* audio_frame(void) {
    if (start == end) {
        end += g_audio_ctx.size;
    } else {
        start += g_audio_ctx.size;
        end += g_audio_ctx.size;
    }


    if (end >= 14700) {
        start = 0;
        end = g_audio_ctx.size;
    }

    printf("%lu, %lu\n", start, end);

    memset(g_audio_ctx.data, INT16_MIN, g_audio_ctx.size);


    for (size_t i = 0;i < g_audio_ctx.samplecount;i += MINO_AUDIO_CHANNELS) {
        memcpy(g_audio_ctx.data, &g_sound_piece0->data[start], g_audio_ctx.size);
    }
    return &g_audio_ctx;
}
