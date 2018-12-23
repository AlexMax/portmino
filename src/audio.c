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
#include "frontend.h"

#define MINO_AUDIO_CHANNELS 2
#define MIXER_CHANNELS 8

typedef struct {
    /**
     * True if this channel is active, otherwise false.
     */
    bool active;

    /**
     * The sound that is playing in this channel.
     * 
     * This is not an owning pointer.  Do not modify or free it.
     */
    const sound_t* sound;

    /**
     * Current index of the playing sound.
     */
    size_t position;
} audio_mixer_channel_t;

sound_t* g_sound_piece0;

static audio_mixer_channel_t g_audio_mixer[MIXER_CHANNELS];
static audio_context_t g_audio_ctx;

/**
 * Reset a mixer channel.
 */
static void audio_mixer_channel_reset(audio_mixer_channel_t* channel) {
    channel->active = false;
    channel->sound = NULL;
    channel->position = 0;
}

/**
 * Find an empty mixer channel.
 * 
 * Returns true if the mixer has an empty slot and returns the found channel
 * in the first parameter.  Otherwise, returns false.
 */
static bool audio_mixer_find_empty(audio_mixer_channel_t** channel) {
    for (int i = 0;i < MIXER_CHANNELS;i++) {
        if (g_audio_mixer[i].active == false) {
            *channel = &g_audio_mixer[i];
            return true;
        }
    }

    return false;
}

/**
 * Debug the state of the audio mixer.
 */
ATTRIB_UNUSED
static void audio_mixer_debug(void) {
    for (int i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_t* channel = &g_audio_mixer[i];

        printf(" [%d] ", i);
        if (channel->active) {
            printf("Playing %p (%lu/%lu)\n", channel->sound, channel->position, channel->sound->size / sizeof(int16_t));
        } else {
            printf("Inactive.\n");
        }
    }
}

/**
 * Initialize the audio subsystem.
 */
void audio_init(void) {
    // Initialize the mixer
    for (size_t i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_reset(&g_audio_mixer[i]);
    }

    // Initialize the context
    g_audio_ctx.samplecount = MINO_AUDIO_HZ / MINO_FPS;
    g_audio_ctx.samplesize = sizeof(int16_t) * MINO_AUDIO_CHANNELS;
    g_audio_ctx.size = g_audio_ctx.samplecount * g_audio_ctx.samplesize;
    g_audio_ctx.data = malloc(g_audio_ctx.size);

    g_sound_piece0 = sound_new("../res/sfx/piece0.wav");
}

/**
 * Clean up the audio subsystem.
 */
void audio_deinit(void) {
    // Clean up the mixer.
    for (int i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_reset(&g_audio_mixer[i]);
    }

    // Free the context.
    if (g_audio_ctx.data != NULL) {
        free(g_audio_ctx.data);
        g_audio_ctx.data = NULL;
    }
}

/**
 * Insert a sound into the mixer.
 */
void audio_playsound(const sound_t* sound) {
    audio_mixer_channel_t* channel = NULL;
    if (!audio_mixer_find_empty(&channel)) {
        // No more room in the mixer.
        return;
    }

    channel->active = true;
    channel->sound = sound;
    channel->position = 0;
}

/**
 * Mix a game-frame's worth of audio data and pass it back to whatever is
 * playing our audio.
 * 
 * Samples is the number of actual audio samples we would like to request.
 * This is usually 735 (44100Hz / 60fps), but the frontend is free to ask
 * for more to avoid audio buffer underruns.
 */
audio_context_t* audio_frame(size_t samples) {
    size_t units = samples * MINO_AUDIO_CHANNELS;
    if (units != g_audio_ctx.size / sizeof(int16_t)) {
        frontend_fatalerror("Size isn't correct - dynamic audio frame sizes aren't implemented yet");
        return NULL;
    }

    // Start mixing our sounds.
    memset(g_audio_ctx.data, 0, g_audio_ctx.size);
    for(size_t i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_t* channel = &g_audio_mixer[i];
        if (channel->active == false) {
            // Skip inactive channels.
            continue;
        }

        // Determine how much of the sound we want to mix.
        bool done = false;
        size_t totalunits = channel->sound->size / sizeof(int16_t);
        size_t soundunits = units;
        if (channel->position + units >= totalunits) {
            // We've reached the end of the sound.  Clamp the number of bytes
            // we're going to mix and reset the channel when we're done.
            done = true;
            soundunits = totalunits - channel->position;
        }

        // Actually do the mixing.
        for (size_t j = 0;j < soundunits;j++) {
            int16_t* mix = channel->sound->data + channel->position + j;
            int16_t* buf = g_audio_ctx.data + j;

            // We do our actual mixing in 32-bit integers - it's much faster
            // to cast and clip than try not to overflow 16-bit operations.
            int32_t mix32 = *mix;
            int32_t buf32 = *buf;

            buf32 += mix32;
            if (buf32 > INT16_MAX) {
                buf32 = INT16_MAX;
            } else if (buf32 < INT16_MIN) {
                buf32 = INT16_MIN;
            }

            *buf = (int16_t)buf32;
        }

        // Advance the position of the sound.
        channel->position += soundunits;

        if (done) {
            // We've played all of the sound.  Clear this channel.
            audio_mixer_channel_reset(channel);
        }
    }

    return &g_audio_ctx;
}
