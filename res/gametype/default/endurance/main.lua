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

-- Interface constants
local BOARD_X = 18
local BOARD_Y = 42

local NEXT_X_START = 18
local NEXT_Y = (BOARD_Y - 20)

local function init(state)
    return true
end

local function after_frame(state)

end

local function draw(state)
    local board = mino_board.get(1)
    local next = mino_next.get(1)

    mino_render.draw_background()
    mino_render.draw_board({x = BOARD_X, y = BOARD_Y}, board)

    local piece = mino_next.get_next_config(next)
    mino_render.draw_piece({x = NEXT_X_START, y = NEXT_Y}, piece)
end

return {
    init = init,
    state_functions = {
        after_frame = after_frame,
    },
    draw = draw,
}
