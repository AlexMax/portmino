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

#include "define.h"
#include "sound.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dr_wav.h"

#include "error.h"
#include "frontend.h"
#include "vfs.h"

/**
 * Convert sound data to our samplerate
 */
static void sound_to_samplerate(int16_t* dst, const int16_t* src, uint64_t srclen) {
    for (uint64_t i = 0, j = 0;i < srclen;i+=2,j+=4) {
        dst[j] = src[i];
        dst[j+1] = src[i+1];
        dst[j+2] = src[i];
        dst[j+3] = src[i+1];
    }
}

/**
 * Convert sound data to stereo
 */
static void sound_to_stereo(int16_t* dst, const int16_t* src, uint64_t srclen) {
    for (uint64_t i = 0, j = 0;i < srclen;i++,j+=2) {
        dst[j] = src[i];
        dst[j + 1] = src[i];
    }
}

/**
 * Allocate a new sound structure.
 */
sound_t* sound_new(const char* path) {
    unsigned int channels;
    unsigned int samplerate;
    uint64_t framecount;

    vfile_t* file = vfs_vfile_new(path);
    if (file == NULL) {
        error_push("Could not find sound %s.", path);
        return NULL;
    }

    int16_t* data = drwav_open_memory_and_read_s16(file->data, file->size,
        &channels, &samplerate, &framecount);

    if (false) {
        printf("Loading %s: size %zu, channels %u, samplerate %u, framecount %lu\n", path, file->size, channels, samplerate, framecount);
    }

    vfs_vfile_delete(file);
    if (data == NULL) {
        error_push("Could not load sound %s.", path);
        return NULL;
    }

    if (channels == 1) {
        // Turn mono into stereo.
        int16_t* dest = reallocarray(NULL, framecount * MINO_AUDIO_CHANNELS, sizeof(int16_t));
        if (dest == NULL) {
            free(data);
            return NULL;
        }

        sound_to_stereo(dest, data, framecount);
        free(data);
        data = dest;
        channels = 2;
    } else if (channels != 2) {
        // We can't convert multi-channel audio.
        free(data);
        return NULL;
    }

    if (samplerate != MINO_AUDIO_HZ) {
        // Convert 22050Hz to 44100Hz.
        int16_t* dest = reallocarray(NULL, framecount * MINO_AUDIO_CHANNELS * 2, sizeof(int16_t));
        if (dest == NULL) {
            free(data);
            return NULL;
        }

        sound_to_samplerate(dest, data, framecount * MINO_AUDIO_CHANNELS);
        free(data);
        data = dest;
        samplerate = MINO_AUDIO_HZ;
        framecount *= 2;
    }

    sound_t* sound = calloc(1, sizeof(sound_t));
    if (sound == NULL) {
        return NULL;
    }

    sound->name = strdup(path);
    if (sound->name == NULL) {
        sound_delete(sound);
        return NULL;
    }

    sound->sampledata = data;
    sound->framecount = framecount;

    return sound;
}

/**
 * Free a sound
 */
void sound_delete(sound_t* sound) {
    if (sound == NULL) {
        return;
    }

    free(sound->name);
    free(sound->sampledata); /* drwav_free is just "free" */
    free(sound);
}
