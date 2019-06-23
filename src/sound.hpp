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

#include "define.hpp"

typedef struct sound_s {
    /**
     * Filename of the sound.
     */
    char* name;

    /**
     * Sound data.
     * 
     * PCM-encoded, 2-channel, 44,100hz.
     */
    int16_t* sampledata;

    /**
     * Number of frames in the sound data.
     */
    size_t framecount;
} sound_t;

sound_t* sound_new(const char* path);
void sound_delete(sound_t* sound);
