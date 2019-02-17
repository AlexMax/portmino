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

/**
 * Push a generic memory allocation error
 * 
 * These are needed in a number of spots, to the point where we need some
 * way to disambiguate them.
 */
#define error_push_allocerr() \
    error_push("%s:%d Memory allocation error.", __FILE__, __LINE__)

ATTRIB_PRINTF(1, 2)
void error_push(const char* fmt, ...);
char* error_pop(void);
size_t error_count(void);
