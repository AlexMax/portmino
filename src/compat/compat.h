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

#include <stddef.h>

// These are only available with C99 compilers or modern MSVC
#include <stdbool.h>
#include <stdint.h>

#if defined(_MSC_VER)
#define restrict __restrict
#endif

#if !defined(HAVE_ASPRINTF)
int asprintf(char** ret, const char* format, ...);
#endif

#if !defined(HAVE_REALLOCARRAY)
void* reallocarray(void* optr, size_t nmemb, size_t size);
#endif
