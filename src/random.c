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

#include <stdlib.h>

#include "mpack.h"

#include "entity.h"
#include "error.h"
#include "frontend.h"
#include "platform.h"
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
 * Allocate a new random number generator
 * 
 * Pass NULL for the parameter if you want a completely random seed, or pass
 * a pointer to the seed for a specific seed.
 * 
 * NOTE: This function is _not safe_ for any cryptographic use.  Half of
 *       the seed is hardcoded, the other half might be generated using
 *       libc rand as a fallback.
 */
random_t* random_new(uint32_t* seed) {
    random_t* random = calloc(1, sizeof(random_t));
    if (random == NULL) {
        error_push_allocerr();
        goto fail;
    }

    // Only half the seed is variable, so we can actually show it without
    // driving people crazy.
    random->state[1] = 0x12EBE5E;
    if (seed == NULL) {
        if (!platform()->random_get_seed(&(random->state[0]))) {
            error_push("Unable to obtain a random seed.");
            goto fail;
        }
    } else {
        random->state[0] = *seed;
    }

    return random;

fail:
    random_delete(random);
    return NULL;
}

/**
 * Free a random number generator
 */
void random_delete(random_t* random) {
    if (random == NULL) {
        return;
    }

    free(random);
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

/**
 * Serialize random struct using msgpack
 */
void random_serialize(random_t* random, mpack_writer_t* writer) {
    mpack_start_array(writer, 2);
    mpack_write_u32(writer, random->state[0]);
    mpack_write_u32(writer, random->state[1]);
    mpack_finish_array(writer);
}

/**
 * Unserialize random struct using msgpack
 */
random_t* random_unserialize(mpack_reader_t* reader) {
    random_t* random = NULL;

    if ((random = calloc(1, sizeof(*random))) == NULL) {
        error_push_allocerr();
        goto fail;
    }

    mpack_expect_array_match(reader, 2);
    random->state[0] = mpack_expect_u32(reader);
    random->state[1] = mpack_expect_u32(reader);
    mpack_done_array(reader);

    return random;

fail:
    return NULL;
}

/**
 * Wrap serialize with void* function.
 */
static void wrapserialize(void* ptr, mpack_writer_t* writer) {
    random_serialize(ptr, writer);
}

/**
 * Wrap delete with void* function.
 */
static void wrapdelete(void* ptr) {
    random_delete(ptr);
}

/**
 * Initialize an entity with random config
 */
void random_entity_init(entity_t* entity) {
    memset(entity, 0x00, sizeof(*entity));

    entity->config.type = MINO_ENTITY_RANDOM;
    entity->config.metatable = "random_t";
    entity->config.serialize = wrapserialize;
    entity->config.destruct = wrapdelete;
}
