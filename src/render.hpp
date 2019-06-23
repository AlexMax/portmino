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

#include "define.hpp"

// Forward declarations
typedef struct board_s board_t;
typedef struct piece_config_s piece_config_t;

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
     * Draw a background image.
     */
    void(*draw_background)(void);

    /**
     * Draw a board and any attached pieces to the screen.
     */
    void (*draw_board)(vec2i_t pos, const board_t* board);

    /**
     * Draw a box to the screen.
     */
    void(*draw_box)(vec2i_t pos, vec2i_t len);

    /**
     * Draw text on the screen.
     */
    void (*draw_font)(vec2i_t pos, const char* text);

    /**
     * Draw the main menu background image.
     */
    void(*draw_mainmenu_bg)(void);

    /**
     * Draw a freestanding piece.
     */
    void (*draw_piece)(vec2i_t pos, const piece_config_t* piece);
} render_module_t;

bool render_init(renderer_type_t renderer);
void render_deinit(void);
render_module_t* render(void);
