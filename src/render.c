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

#include "render.h"

#include <stdlib.h>
#include <string.h>

#include "softrender.h"

/**
 * Initialize a renderer and return the function pointers for that renderer.
 */
render_module_t* render_init(void) {
    // TODO: Figure out what kind of renderer that we want to use here.
    soft_render_module.init();
    return &soft_render_module;
}

/**
 * Destroy anything attached to the currently initialized renderer.
 */
void render_deinit(render_module_t* module) {
    (void)module;
    soft_render_module.deinit();
}
