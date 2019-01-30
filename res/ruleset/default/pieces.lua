-- This file is part of Portmino.
-- 
-- Portmino is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
-- 
-- Portmino is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License
-- along with Portmino.  If not, see <https://www.gnu.org/licenses/>.

-- Piece definitions
local j_piece = {
    name = "J",
    data = {
        8, 0, 0,
        8, 8, 8,
        0, 0, 0,
        0, 8, 8,
        0, 8, 0,
        0, 8, 0,
        0, 0, 0,
        8, 8, 8,
        0, 0, 8,
        0, 8, 0,
        0, 8, 0,
        8, 8, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 3,
    height = 3
}

local l_piece = {
    name = "L",
    data = {
        0, 0, 4,
        4, 4, 4,
        0, 0, 0,
        0, 4, 0,
        0, 4, 0,
        0, 4, 4,
        0, 0, 0,
        4, 4, 4,
        4, 0, 0,
        4, 4, 0,
        0, 4, 0,
        0, 4, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 3,
    height = 3
}

local s_piece = {
    name = "S",
    data = {
        0, 6, 6,
        6, 6, 0,
        0, 0, 0,
        0, 6, 0,
        0, 6, 6,
        0, 0, 6,
        0, 0, 0,
        0, 6, 6,
        6, 6, 0,
        6, 0, 0,
        6, 6, 0,
        0, 6, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 3,
    height = 3,
}

local z_piece = {
    name = "Z",
    data = {
        3, 3, 0,
        0, 3, 3,
        0, 0, 0,
        0, 0, 3,
        0, 3, 3,
        0, 3, 0,
        0, 0, 0,
        3, 3, 0,
        0, 3, 3,
        0, 3, 0,
        3, 3, 0,
        3, 0, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 3,
    height = 3
}

local t_piece = {
    name = "T",
    data = {
        0, 9, 0,
        9, 9, 9,
        0, 0, 0,
        0, 9, 0,
        0, 9, 9,
        0, 9, 0,
        0, 0, 0,
        9, 9, 9,
        0, 9, 0,
        0, 9, 0,
        9, 9, 0,
        0, 9, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 3,
    height = 3
}

local i_piece = {
    name = "I",
    data = {
        0, 0, 0, 0,
        7, 7, 7, 7,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 7, 0,
        0, 0, 7, 0,
        0, 0, 7, 0,
        0, 0, 7, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        7, 7, 7, 7,
        0, 0, 0, 0,
        0, 7, 0, 0,
        0, 7, 0, 0,
        0, 7, 0, 0,
        0, 7, 0, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 4,
    height = 4
}

local o_piece = {
    name = "O",
    data = {
        0, 5, 5, 0,
        0, 5, 5, 0,
        0, 0, 0, 0,
        0, 5, 5, 0,
        0, 5, 5, 0,
        0, 0, 0, 0,
        0, 5, 5, 0,
        0, 5, 5, 0,
        0, 0, 0, 0,
        0, 5, 5, 0,
        0, 5, 5, 0,
        0, 0, 0, 0
    },
    spawn_pos = { 3, 1 },
    spawn_rot = 0,
    width = 4,
    height = 3
}

return {
    j_piece, l_piece, s_piece, z_piece, t_piece, i_piece, o_piece,
}
