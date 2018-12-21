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

#include <stdlib.h>
#include <string.h>

#include "audio.h"

#define MINO_AUDIO_CHANNELS 2

static audio_context_t g_audio_ctx;

/**
 * Initialize the audio context.
 */
void audio_init(void) {
    g_audio_ctx.samplecount = MINO_AUDIO_HZ / MINO_FPS;
    g_audio_ctx.samplesize = sizeof(int16_t) * MINO_AUDIO_CHANNELS;
    g_audio_ctx.size = g_audio_ctx.samplecount * g_audio_ctx.samplesize;
    g_audio_ctx.data = malloc(g_audio_ctx.size);
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

/**
 * Set up a game-frame's worth of audio data and pass it back to whatever
 * is playing our audio.
 */
audio_context_t* audio_frame(void) {
    memset(g_audio_ctx.data, INT16_MIN, g_audio_ctx.size);
    for (size_t i = 0;i < g_audio_ctx.samplecount;i += MINO_AUDIO_CHANNELS) {
        int16_t x = INT16_MAX;
        g_audio_ctx.data[i] = x;
        g_audio_ctx.data[i + 1] = x;
    }
    return &g_audio_ctx;
}
