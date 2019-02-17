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

#include "error.h"

#include <stdarg.h>
#include <stdio.h>

#define MINO_MAX_ERRORS 4
#define MINO_MAX_ERROR_LEN 1024

/**
 * Error messages
 */
static char g_errors[MINO_MAX_ERRORS][MINO_MAX_ERROR_LEN];

/**
 * Number of pushed errors
 */
static size_t g_error_count = 0;

/**
 * Front of the queue
 */
static size_t g_error_front = 0;

/**
 * Back of the queue
 */
static size_t g_error_back = 0;

/*
 * Debug the error queue
 */
static void error_debug(void) {
    fprintf(stderr, "-- front: %zu, back: %zu, count: %zu --\n", g_error_front,
            g_error_back, g_error_count);
    for (size_t i = 0;i < MINO_MAX_ERRORS;i++) {
        if (g_error_front == i) {
            fprintf(stderr, ">");
        } else {
            fprintf(stderr, " ");
        }
        fprintf(stderr, " queue[%zu]: %s\n", i, &g_errors[i][0]);
    }
}

/**
 * Push an error message
 */
void error_push(const char* fmt, ...) {
    // Push the error to the back
    va_list va;
    va_start(va, fmt);
    vsnprintf(&g_errors[g_error_back][0], MINO_MAX_ERROR_LEN, fmt, va);
    va_end(va);

    // Advance the error count
    if (g_error_count < MINO_MAX_ERRORS) {
        // We can grow the error count by one
        g_error_count += 1;
    } else {
        // We just paved over an existing error, advance the front
        g_error_front = (g_error_front + 1) % MINO_MAX_ERRORS;
    }

    // The new back
    g_error_back = (g_error_back + 1) % MINO_MAX_ERRORS;
}

/**
 * Read an error message and mark it as read
 */
char* error_pop(void) {
    error_debug();

    if (g_error_count == 0) {
        // No errors to pop
        return NULL;
    }

    // Get the error index to return
    size_t index = g_error_front;

    // Advance the front of the queue to the next item
    g_error_front = (g_error_front + 1) % MINO_MAX_ERRORS;

    // Decrease the item count by one
    g_error_count -= 1;

    return &g_errors[index][0];
}

/**
 * Number of errors in the queue
 */
size_t error_count(void) {
    return g_error_count;
}
