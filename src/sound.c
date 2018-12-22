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

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "sound.h"

/**
 * Convert sound data to our samplerate
 */
static void sound_to_samplerate(int16_t* dst, int16_t* src, uint64_t srclen) {
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
static void sound_to_stereo(int16_t* dst, int16_t* src, uint64_t srclen) {
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

    int16_t* data = drwav_open_file_and_read_pcm_frames_s16(path,
        &channels, &samplerate, &framecount);

    if (channels == 1) {
        // Turn mono into stereo.
        int16_t* dest = malloc(sizeof(int16_t) * framecount * 2);
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
        int16_t* dest = malloc(sizeof(int16_t) * framecount * 2);
        if (dest == NULL) {
            free(data);
            return NULL;
        }

        sound_to_samplerate(dest, data, framecount);
        free(data);
        data = dest;
        samplerate = MINO_AUDIO_HZ;
        framecount *= 2;
    }

    sound_t* sound = malloc(sizeof(sound_t));
    if (sound == NULL) {
        return NULL;
    }

    sound->data = data;
    sound->size = channels * framecount;

    return sound;
}

/**
 * Free a sound
 */
void sound_delete(sound_t* sound) {
    if (sound->data != NULL) {
        free(sound->data); /* drwav_free is just "free" */
    }

    free(sound);
}
