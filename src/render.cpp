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

#include "render.hpp"

#include <stdlib.h>
#include <string.h>

#include "error.hpp"
#include "softrender.hpp"

static render_module_t g_render_module;

/**
 * Initialize the renderer.
 */
bool render_init(renderer_type_t renderer) {
    switch (renderer) {
    case RENDERER_SOFTWARE:
        memcpy(&g_render_module, &softrender_module, sizeof(render_module_t));
        break;
    }

    return g_render_module.init();
}

/**
 * Destroy the active renderer.
 */
void render_deinit(void) {
   g_render_module.deinit();
}

/**
 * Return a pointer to the current render module.
 */
render_module_t* render(void) {
    return &g_render_module;
}
