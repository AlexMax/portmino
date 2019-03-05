-- Bag randomizer
local function next_piece(board_id)
    local state = mino_state.get()
    local board = state.board[board_id]

    -- If our bag is empty, fill it back up.
    if board.bag_size == 0 then
        board.bag = mino_ruleset.get_piece_configs()
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

return next_piece
