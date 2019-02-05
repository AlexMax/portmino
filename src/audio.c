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

#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frontend.h"
#include "sound.h"

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
     * Current frame of the playing sound.
     */
    size_t frame;
} audio_mixer_channel_t;

sound_t* g_sound_cursor;
sound_t* g_sound_gameover;
sound_t* g_sound_lock;
sound_t* g_sound_move;
sound_t* g_sound_ok;
sound_t* g_sound_piece0;
sound_t* g_sound_rotate;
sound_t* g_sound_step;

static audio_mixer_channel_t g_audio_mixer[MIXER_CHANNELS];
static audio_context_t g_audio_ctx;

/**
 * Reset a mixer channel.
 */
static void audio_mixer_channel_reset(audio_mixer_channel_t* channel) {
    channel->active = false;
    channel->sound = NULL;
    channel->frame = 0;
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

        if (channel->active) {
            fprintf(stderr, " [%d] Playing %s (%lu/%lu)\n", i, channel->sound->name, channel->frame, channel->sound->framecount);
        } else {
            // fprintf(stderr, "Inactive.\n");
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
    g_audio_ctx.framecount = MINO_AUDIO_HZ / MINO_FPS;
    g_audio_ctx.sizeofframe = sizeof(int16_t) * MINO_AUDIO_CHANNELS;
    g_audio_ctx.bytesize = g_audio_ctx.framecount * g_audio_ctx.sizeofframe;
    g_audio_ctx.sampledata = malloc(g_audio_ctx.bytesize);

    g_sound_cursor = sound_new("sfx/default/cursor.wav");
    g_sound_gameover = sound_new("sfx/default/gameover.wav");
    g_sound_lock = sound_new("sfx/default/lock.wav");
    g_sound_move = sound_new("sfx/default/move.wav");
    g_sound_ok = sound_new("sfx/default/ok.wav");
    g_sound_piece0 = sound_new("sfx/default/piece0.wav");
    g_sound_rotate = sound_new("sfx/default/rotate.wav");
    g_sound_step = sound_new("sfx/default/step.wav");
}

/**
 * Clean up the audio subsystem.
 */
void audio_deinit(void) {
    // Clean up the mixer.
    for (int i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_reset(&g_audio_mixer[i]);
    }

    // Delete all sounds
    if (g_sound_cursor != NULL) {
        sound_delete(g_sound_cursor);
        g_sound_cursor = NULL;
    }
    if (g_sound_gameover != NULL) {
        sound_delete(g_sound_gameover);
        g_sound_gameover = NULL;
    }
    if (g_sound_lock != NULL) {
        sound_delete(g_sound_lock);
        g_sound_lock = NULL;
    }
    if (g_sound_move != NULL) {
        sound_delete(g_sound_move);
        g_sound_move = NULL;
    }
    if (g_sound_ok != NULL) {
        sound_delete(g_sound_ok);
        g_sound_ok = NULL;
    }
    if (g_sound_piece0 != NULL) {
        sound_delete(g_sound_piece0);
        g_sound_piece0 = NULL;
    }
    if (g_sound_rotate != NULL) {
        sound_delete(g_sound_rotate);
        g_sound_rotate = NULL;
    }
    if (g_sound_step != NULL) {
        sound_delete(g_sound_step);
        g_sound_step = NULL;
    }

    // Free the context.
    if (g_audio_ctx.sampledata != NULL) {
        free(g_audio_ctx.sampledata);
        g_audio_ctx.sampledata = NULL;
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
    channel->frame = 0;
}

/**
 * Mix a game-frame's worth of audio data and pass it back to whatever is
 * playing our audio.
 * 
 * Frames is the number of actual audio samples we would like to request.
 * This is usually 735 (44100Hz / 60fps), but the frontend is free to ask
 * for more to avoid audio buffer underruns.
 */
audio_context_t* audio_frame(size_t frames) {
    if (frames != MINO_AUDIO_HZ / MINO_FPS) {
        frontend_fatalerror("Size isn't correct - dynamic audio frame sizes aren't implemented yet");
        return NULL;
    }

    // Start mixing our sounds.
    memset(g_audio_ctx.sampledata, 0, g_audio_ctx.bytesize);
    for(size_t i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_t* channel = &g_audio_mixer[i];
        if (channel->active == false) {
            // Skip inactive channels.
            continue;
        }

        // Determine how much of the sound we want to mix.
        bool done = false;
        size_t totalframes = channel->sound->framecount;
        size_t mixframes = frames;
        if (channel->frame + frames >= totalframes) {
            // We've reached the end of the sound.  Clamp the number of bytes
            // we're going to mix and reset the channel when we're done.
            done = true;
            mixframes = totalframes - channel->frame;
        }

        // Actually do the mixing.  Note that here, and only here, we address
        // individual samples instead of whole frames.
        size_t mixsamples = mixframes * MINO_AUDIO_CHANNELS;
        for (size_t s = 0;s < mixsamples;s++) {
            size_t soff = channel->frame * MINO_AUDIO_CHANNELS;
            int16_t* mix = channel->sound->sampledata + soff + s;
            int16_t* buf = g_audio_ctx.sampledata + s;

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
        channel->frame += mixframes;

        if (done) {
            // We've played all of the sound.  Clear this channel.
            audio_mixer_channel_reset(channel);
        }
    }

    return &g_audio_ctx;
}
