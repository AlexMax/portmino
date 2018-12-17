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

/**
 * This file uses an implementation of xoroshiro64** 1.0 by David Blackman
 * and Sebastiano Vigna, licensed under CC0.
 */

#include <stdbool.h>
#include <stdlib.h>

#include "random.h"

/**
 * Rotate left
 */
static inline uint32_t rotl(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

/**
 * Return a random 32-bit number from the current state and mutate the state
 * in preparation for the next one.
 */
static uint32_t random_next(random_t* random) {
    const uint32_t s0 = random->state[0];
    uint32_t s1 = random->state[1];
    const uint32_t result_starstar = rotl(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
    random->state[0] = rotl(s0, 26) ^ s1 ^ (s1 << 9); // a, b
    random->state[1] = rotl(s1, 13); // c

    return result_starstar;
}

/**
 * Get a 32-bit random seed for the random number generator.
 */

#if defined(__linux__)
#include <sys/random.h>

static bool random_get_seed(uint32_t* seed) {
    ssize_t ok = getrandom(seed, sizeof(uint32_t), GRND_NONBLOCK);
    if (ok == -1) {
        return false;
    }
    return true;
}

#else
#error "Please implement a source of 32-bit random seed for your platform"
#endif

/**
 * Initialize a new random number generator state inplace, without allocation.
 * 
 * Pass NULL for the second parameter if you want a completely random seed,
 * or pass a pointer to the seed for a specific seed.
 */
void random_init(random_t* random, uint32_t* seed) {
    // Only half the seed is variable, so we can actually show it.
    random->state[1] = 0x12EBE5E;
    if (seed == NULL) {
        if (!random_get_seed(&(random->state[0]))) {
            // FIXME: Don't freakin' kill the whole process because you can't
            //        get a random number.
            abort();
        }
    } else {
        random->state[0] = *seed;
    }
}

/**
 * Get a random number in a given range, without bias.
 * 
 * Taken from OpenBSD.  Would rather not introduce any fencepost errors or
 * hidden bias by coming up with it from whole cloth.
 */
uint32_t random_number(random_t* random, uint32_t range) {
    uint32_t t = (-range) % range;
    for (;;) {
        uint32_t r = random_next(random);
        if (r >= t) {
            return r % range;
        }
    }
}
