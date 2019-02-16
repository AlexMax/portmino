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

#define MAX_ERRORS 4
#define MAX_ERROR_LEN 1024

/**
 * Error messages
 */
static char g_errors[MAX_ERRORS][MAX_ERROR_LEN];

/**
 * Number of pushed errors
 */
static size_t g_error_count = 0;

/**
 * Error index
 */
static size_t g_error_head = MAX_ERRORS - 1;

/**
 * Push an error message
 */
void error_push(const char* fmt, ...) {
    // Advance the head of the list
    g_error_head = (g_error_head + 1) % MAX_ERRORS;

    // Push the error
    va_list va;
    va_start(va, fmt);
    vsnprintf(&g_errors[g_error_head][0], MAX_ERROR_LEN, fmt, va);
    va_end(va);

    // Advance the error count
    if (g_error_count <= MAX_ERRORS) {
        g_error_count += 1;
    }
}

/**
 * Read an error message and mark it as read.
 */
char* error_pop(void) {
    if (g_error_count == 0) {
        // No errors to pop
        return NULL;
    }

    // Get the error index to return
    size_t index = (MAX_ERRORS + g_error_head - g_error_count + 1) % MAX_ERRORS;

    // Pop the error
    g_error_count -= 1;

    return &g_errors[index][0];
}
