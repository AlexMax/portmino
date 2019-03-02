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

#include "define.h"

// Forward declarations
typedef struct state_s state_t;

typedef enum {
    RENDERER_SOFTWARE
} renderer_type_t;

typedef struct {
    /**
     * Type of renderer
     */
    int type;

    /**
     * This function is run when the renderer is initialized.
     */
    bool (*init)(void);

    /**
     * This function is run when the renderer is destroyed.
     */
    void (*deinit)(void);

    /**
     * This function is used to return the rendering context.
     */
    void* (*context)(void);

    /**
     * Clear everything from the screen.
     */
    void (*clear)(void);

    /**
     * Draw the main menu background image.
     */
    void (*draw_mainmenu_bg)(void);

    /**
     * This function draws text on the screen.
     */
    void (*draw_font)(vec2i_t pos, const char* text);

    /**
     * This function is run once pre frame to actually do the drawing.
     */
    void (*draw_state)(const state_t* state);
} render_module_t;

bool render_init(renderer_type_t renderer);
void render_deinit(void);
render_module_t* render(void);
