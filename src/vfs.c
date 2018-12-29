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

#include "vfs.h"

void vfs_init(void) {

}

void vfs_deinit(void) {

}

/**
 * Obtain file data by virtual filename.
 * 
 * If there is an error retriving file data, the returned pointer will be NULL.
 */
filedata_t* vfs_file(const char* filename) {
    filedata_t data = { 0 };

    return &data;
}
