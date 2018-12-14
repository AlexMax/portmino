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

static piece_config_t s_piece;

piece_t* piece_new_s(void) {
    if (s_piece.datas == NULL) {
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

        s_piece.width = 3;
        s_piece.height = 3;
        s_piece.data_size = s_piece.width * s_piece.height;
        s_piece.data_count = 4;
        s_piece.datas = malloc(sizeof(uint8_t) * s_piece.data_count * s_piece.data_size);
        memcpy(s_piece.datas, &data, sizeof(data));
    }

    piece_t* piece = malloc(sizeof(piece_t));

    piece->config = &s_piece;
    piece->rot = 3;
    piece->x = 0;
    piece->y = 0;

    return piece;
}

void piece_delete(piece_t* piece) {
    free(piece);
}
