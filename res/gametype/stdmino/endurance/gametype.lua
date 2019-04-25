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

local TEXT_X = 160
local TEXT_Y = BOARD_Y

local NEXT_X_START = 18
local NEXT_Y = (BOARD_Y - 20)

local function draw(state)
    local board = state.board[1]

    -- Draw the actual game
    mino_render.draw_background()
    mino_render.draw_board({x = BOARD_X, y = BOARD_Y}, board.board)

    -- Draw the next pieces
    mino_render.draw_piece({x = NEXT_X_START, y = NEXT_Y}, state.board[1].next[1])
    mino_render.draw_piece({x = NEXT_X_START + 36, y = NEXT_Y}, state.board[1].next[2])
    mino_render.draw_piece({x = NEXT_X_START + 72, y = NEXT_Y}, state.board[1].next[3])

    -- Draw the HUD
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y}, "Marathon")
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 1)}, "(150 Line Game)")

    local score = string.format("%d", state.player[1].score)
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 3)}, "Score")
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 4)}, score)

    local lines = string.format("%d", state.player[1].lines)
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 6)}, "Lines")
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 7)}, lines)

    --local level = string.format("%d", state.player[1].level)
    --mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 9)}, "Level")
    --mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 10)}, level)

    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 12)}, "Time")
    mino_render.draw_font({x = TEXT_X, y = TEXT_Y + (8 * 13)}, "00:00.00")
end

return {
    draw = draw,
}
