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

local randomizer = require('randomizer')

local NEXT_BUFFER_SIZE = 3

-- Initialize the next piece buffer
local function init_next(board)
    for i = 1, NEXT_BUFFER_SIZE do
        board.next[i] = randomizer.next_piece(board)
    end
end

-- Peek at the next piece
local function peek_next(board)
    return board.next[1]
end

-- Consume the current next piece
local function consume_next(board)
    -- Remove the head of the table 
    table.remove(board.next, 1)

    -- Grab another piece out of the bag and put it into tail of the table
    board.next[NEXT_BUFFER_SIZE] = randomizer.next_piece(board)
end

return {
    init_next = init_next,
    peek_next = peek_next,
    consume_next = consume_next
}
