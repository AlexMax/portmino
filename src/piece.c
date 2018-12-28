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

#include <stdlib.h>
#include <string.h>

#include "piece.h"

/**
 * Default piece configurations.
 */

piece_config_t g_j_piece;
piece_config_t g_l_piece;
piece_config_t g_s_piece;
piece_config_t g_z_piece;
piece_config_t g_t_piece;
piece_config_t g_i_piece;
piece_config_t g_o_piece;

/**
 * Initialize the J piece.
 */
static void piece_config_init_j(piece_config_t* piece) {
    uint8_t data[] = {
        6, 0, 0,
        6, 6, 6,
        0, 0, 0,
        0, 6, 6,
        0, 6, 0,
        0, 6, 0,
        0, 0, 0,
        6, 6, 6,
        0, 0, 6,
        0, 6, 0,
        0, 6, 0,
        6, 6, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 3;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the L piece.
 */
static void piece_config_init_l(piece_config_t* piece) {
    uint8_t data[] = {
        0, 0, 2,
        2, 2, 2,
        0, 0, 0,
        0, 2, 0,
        0, 2, 0,
        0, 2, 2,
        0, 0, 0,
        2, 2, 2,
        2, 0, 0,
        2, 2, 0,
        0, 2, 0,
        0, 2, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 3;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the S piece.
 */
static void piece_config_init_s(piece_config_t* piece) {
    uint8_t data[] = {
        0, 4, 4,
        4, 4, 0,
        0, 0, 0,
        0, 4, 0,
        0, 4, 4,
        0, 0, 4,
        0, 0, 0,
        0, 4, 4,
        4, 4, 0,
        4, 0, 0,
        4, 4, 0,
        0, 4, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 3;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the Z piece.
 */
static void piece_config_init_z(piece_config_t* piece) {
    uint8_t data[] = {
        1, 1, 0,
        0, 1, 1,
        0, 0, 0,
        0, 0, 1,
        0, 1, 1,
        0, 1, 0,
        0, 0, 0,
        1, 1, 0,
        0, 1, 1,
        0, 1, 0,
        1, 1, 0,
        1, 0, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 3;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the T piece.
 */
static void piece_config_init_t(piece_config_t* piece) {
    uint8_t data[] = {
        0, 7, 0,
        7, 7, 7,
        0, 0, 0,
        0, 7, 0,
        0, 7, 7,
        0, 7, 0,
        0, 0, 0,
        7, 7, 7,
        0, 7, 0,
        0, 7, 0,
        7, 7, 0,
        0, 7, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 3;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the I piece.
 */
static void piece_config_init_i(piece_config_t* piece) {
    uint8_t data[] = {
        0, 0, 0, 0,
        5, 5, 5, 5,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 5, 0,
        0, 0, 5, 0,
        0, 0, 5, 0,
        0, 0, 5, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        5, 5, 5, 5,
        0, 0, 0, 0,
        0, 5, 0, 0,
        0, 5, 0, 0,
        0, 5, 0, 0,
        0, 5, 0, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 4;
    piece->height = 4;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Initialize the O piece.
 */
static void piece_config_init_o(piece_config_t* piece) {
    uint8_t data[] = {
        0, 3, 3, 0,
        0, 3, 3, 0,
        0, 0, 0, 0,
        0, 3, 3, 0,
        0, 3, 3, 0,
        0, 0, 0, 0,
        0, 3, 3, 0,
        0, 3, 3, 0,
        0, 0, 0, 0,
        0, 3, 3, 0,
        0, 3, 3, 0,
        0, 0, 0, 0
    };

    piece->spawn_pos.x = 3;
    piece->spawn_pos.y = 1;
    piece->spawn_rot = 0;
    piece->width = 4;
    piece->height = 3;
    piece->data_size = piece->width * piece->height;
    piece->data_count = 4;
    piece->datas = malloc(sizeof(uint8_t) * piece->data_count * piece->data_size);
    memcpy(piece->datas, &data, sizeof(data));
}

/**
 * Clean up a piece.
 */
static void piece_config_deinit(piece_config_t* piece) {
    free(piece->datas);
}

/**
 * Initialize all our pieces.
 */
void pieces_init(void) {
    piece_config_init_j(&g_j_piece);
    piece_config_init_l(&g_l_piece);
    piece_config_init_s(&g_s_piece);
    piece_config_init_z(&g_z_piece);
    piece_config_init_t(&g_t_piece);
    piece_config_init_i(&g_i_piece);
    piece_config_init_o(&g_o_piece);
}

/**
 * Clean up our pieces.
 */
void pieces_deinit(void) {
    piece_config_deinit(&g_j_piece);
    piece_config_deinit(&g_l_piece);
    piece_config_deinit(&g_s_piece);
    piece_config_deinit(&g_z_piece);
    piece_config_deinit(&g_t_piece);
    piece_config_deinit(&g_i_piece);
    piece_config_deinit(&g_o_piece);
}

/**
 * Initialize a new piece on the board.
 */
piece_t* piece_new(const piece_config_t* config) {
    piece_t* piece = malloc(sizeof(piece_t));

    piece->config = config;
    piece->rot = config->spawn_rot;
    piece->pos = config->spawn_pos;

    return piece;
}

/**
 * Delete a piece on the board.
 */
void piece_delete(piece_t* piece) {
    free(piece);
}

/**
 * Get the initial data position of a particular rotation.
 */
uint8_t* piece_get_rot(const piece_config_t* piece, uint8_t rot) {
    return piece->datas + (rot * piece->data_size);
}
