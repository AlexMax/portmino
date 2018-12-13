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

#include "state.h"

/*
 * Here we define the width and height of the smallest unit of addressable
 * screen real-estate.  This has little to do with the actual resolution of
 * the rendered screen, aside from serving as a lower bound.
 */

#define RENDER_WIDTH 320
#define RENDER_HEIGHT 240

enum {
    RENDERER_SOFTWARE
};

typedef struct {
    /**
     * Type of renderer
     */
    int type;

    /**
     * This function is run when the renderer is initialized.
     */
    void (*init)(void);

    /**
     * This function is run when the renderer is destroyed.
     */
    void (*deinit)(void);

    /**
     * This function is run once pre frame to actually do the drawing.
     */
    void* (*draw_state)(const state_t* state);
} render_module_t;

render_module_t* render_init(void);
void render_deinit(render_module_t* module);
