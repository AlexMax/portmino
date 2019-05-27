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

#include "khash.h"

#include "error.h"
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

KHASH_MAP_INIT_STR(g_sounds, sound_t*);
khash_t(g_sounds)* g_sounds;

static audio_mixer_channel_t g_audio_mixer[MIXER_CHANNELS];
static audio_context_t g_audio_ctx;
static bool g_audio_init;

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
            fprintf(stderr, " [%d] Playing %s (%zu/%zu)\n", i, channel->sound->name, channel->frame, channel->sound->framecount);
        } else {
            // fprintf(stderr, "Inactive.\n");
        }
    }
}

/**
 * Initialize the audio subsystem.
 */
bool audio_init(void) {
    // Initialize the mixer
    for (size_t i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_reset(&g_audio_mixer[i]);
    }

    // Allocate our sample data buffer ahead of time, 0.25 of a second should be plenty
    g_audio_ctx.actualbytesize = (MINO_AUDIO_HZ / 4) * sizeof(int16_t) * MINO_AUDIO_CHANNELS;
    if ((g_audio_ctx.sampledata = malloc(g_audio_ctx.actualbytesize)) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Initialize our sound hashmap
    g_sounds = kh_init_g_sounds();

    // Add our base sounds
    if (audio_loadsound("cursor") == false) {
        goto fail;
    }
    if (audio_loadsound("gameover") == false) {
        goto fail;
    }
    if (audio_loadsound("go") == false) {
        goto fail;
    }
    if (audio_loadsound("lock") == false) {
        goto fail;
    }
    if (audio_loadsound("move") == false) {
        goto fail;
    }
    if (audio_loadsound("ok") == false) {
        goto fail;
    }
    if (audio_loadsound("piece0") == false) {
        goto fail;
    }
    if (audio_loadsound("ready") == false) {
        goto fail;
    }
    if (audio_loadsound("rotate") == false) {
        goto fail;
    }
    if (audio_loadsound("step") == false) {
        goto fail;
    }

    g_audio_init = true;
    return true;

fail:
    error_push("Could not initialize audio subsystem");
    return false;
}

/**
 * Clean up the audio subsystem.
 */
void audio_deinit(void) {
    g_audio_init = false;

    // Clean up the mixer.
    for (int i = 0;i < MIXER_CHANNELS;i++) {
        audio_mixer_channel_reset(&g_audio_mixer[i]);
    }

    // Destroy all sounds in the hashtable.
    sound_t* sound = NULL;
    kh_foreach_value(g_sounds, sound, sound_delete(sound));

    // Destroy the sounds hashtable itself.
    kh_destroy_g_sounds(g_sounds);
    g_sounds = NULL;

    // Free the context.
    free(g_audio_ctx.sampledata);
    g_audio_ctx.sampledata = NULL;
}

/**
 * Add a sound by name
 */
bool audio_loadsound(const char* name) {
    char* path = NULL;
    sound_t* sound = NULL;

    int ok = asprintf(&path, "sfx/default/%s.wav", name);
    if (ok < 0) {
        error_push_allocerr();
        goto fail;
    }

    sound = sound_new(path);
    if (sound == NULL) {
        // Error comes from the function
        goto fail;
    }

    int ret;
    khint_t key = kh_put_g_sounds(g_sounds, name, &ret);
    if (ret == 0) {
        // Key already exists
        error_push("Sound \"%s\" is already loaded!", name);
        goto fail;
    } else if (ret == -1) {
        // Operation failed
        error_push_allocerr();
        goto fail;
    }
    kh_val(g_sounds, key) = sound;

    free(path);
    return true;

fail:
    free(path);
    sound_delete(sound);
    return false;
}

/**
 * Insert a sound into the mixer, by name.
 */
void audio_playsound(const char* name) {
    audio_mixer_channel_t* channel = NULL;
    if (!audio_mixer_find_empty(&channel)) {
        // No more room in the mixer.
        return;
    }

    // Look up the sound we're trying to play.
    khint_t key = kh_get_g_sounds(g_sounds, name);
    if (key == kh_end(g_sounds)) {
        // Can't find the sound.
        return;
    }

    channel->active = true;
    channel->sound = kh_value(g_sounds, key);
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
    if (g_audio_init == false) {
        return NULL;
    }

    // Ensure our audio context is the correct size
    if (g_audio_ctx.framecount != frames) {
        g_audio_ctx.framecount = frames;
        g_audio_ctx.sizeofframe = sizeof(int16_t) * MINO_AUDIO_CHANNELS;
        g_audio_ctx.bytesize = g_audio_ctx.framecount * g_audio_ctx.sizeofframe;
        if (g_audio_ctx.bytesize > g_audio_ctx.actualbytesize) {
            // We overran our audio buffer, bail out.
            return NULL;
        }
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
