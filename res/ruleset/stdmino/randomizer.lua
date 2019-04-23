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

-- Actually obtain the random next piece for a particular board
local function next_piece(board)
    -- If our bag is empty, fill it back up.
    if board.bag_size == 0 then
        for key, value in ipairs(pieces) do
            board.bag[key] = mino_piece.new(value)
        end
        board.bag_size = #board.bag
    end

    -- Pick a random piece from the bag.
    local index = board.random:number(board.bag_size) + 1

    -- Iterate that random number of times through the bag.
    local key = nil
    local piece = nil
    for i = 1, index do
        key, piece = next(board.bag, key)
    end

    -- Remove the piece from the bag and return it.
    board.bag[key] = nil
    board.bag_size = board.bag_size - 1
    return piece
end

return {
    next_piece = next_piece
}
