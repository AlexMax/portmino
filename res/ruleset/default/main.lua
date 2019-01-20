-- Stubbing out defines
local STATE_RESULT_OK = 0
local STATE_RESULT_ERROR = 1
local STATE_RESULT_GAMEOVER = 2
local STATE_RESULT_SUCCESS = 3

local EVENT_NONE = 0
local EVENT_LEFT = 1
local EVENT_RIGHT = 1 << 1
local EVENT_SOFTDROP = 1 << 2
local EVENT_HARDDROP = 1 << 3
local EVENT_CCW = 1 << 4
local EVENT_CW = 1 << 5
local EVENT_HOLD = 1 << 6
local EVENT_180 = 1 << 7

local DEFAULT_DAS = 12
local DEFAULT_DAS_PERIOD = 2
local DEFAULT_LOCK_DELAY = 30

local ROT_0 = 0
local ROT_R = 1
local ROT_2 = 2
local ROT_L = 3

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

-- Contains any important state that we need to keep track of.
local state = {
    player = {
        {
            -- Tic that EVENT_LEFT began on.  Set to 0 if released.
            left_tic = 0,

            -- Tic that EVENT_RIGHT began on.  Set to 0 if released.
            right_tic = 0,

            -- Tic that lock delay started on.  Set to 0 if lock delay isn't in effect.
            lock_tic = 0,

            -- Tic that EVENT_HARDDROP began on.  Set to 0 if released.
            harddrop_tic = 0,

            -- Have we processed an EVENT_CCW last tic?
            ccw_already = false,

            -- Have we processed an EVENT_CW last tic?
            cw_already = false,
        }
    },
    board = {
        {
            -- Tic that the current piece spawned on.
            spawn_tic = 0,

            -- Random number generator for this board.
            random = mino_random.new(nil),

            -- Bag of random pieces.
            bag = {},

            -- Size of the bag.
            bag_size = 0,
        }
    }
}

local function board_next_piece(board, tic)
    if mino_board.get_piece(board) ~= nil then
        mino_board.delete_piece(board)
    end

    -- Find the next piece.
    local config = mino_board.get_next_config(board)

    -- See if our newly-spawned piece would collide with an existing piece.
    local spawn_pos = mino_piece.config_get_spawn_pos(config)
    local spawn_rot = mino_piece.config_get_spawn_rot(config)
    if not mino_board.test_piece(board, config, spawn_pos, spawn_rot) then
        spawn_pos.y = spawn_pos.y - 1
        if not mino_board.test_piece(board, config, spawn_pos, spawn_rot) then
            return false
        end

        -- Piece spawns offset from its usual spot
        mino_board.new_piece(board, config)
        local piece = mino_board.get_piece(board)
        mino_piece.set_pos(piece, spawn_pos)
    else
        mino_board.new_piece(board, config)
    end

    -- Set the spawn tic.
    state.board[1].spawn_tic = tic

    -- Advance the next index.
    mino_board.consume_next(board)

    return true
end

local function state_frame()
    local gametic = mino_ruleset.get_gametic()
    local events = mino_ruleset.get_player_events(1)
    local board = mino_board.get(1)

    -- Get the next piece if we don't have one at this point.
    if mino_board.get_piece(board) == nil then
        if not board_next_piece(board, gametic) then
            return STATE_RESULT_GAMEOVER
        end
        mino_audio.playsound("piece0")
    end

    -- Get our piece
    local piece = mino_board.get_piece(board)
    local piece_pos = mino_piece.get_pos(piece)
    local piece_rot = mino_piece.get_rot(piece)
    local piece_config = mino_piece.get_config(piece)

    -- Is our piece blocked from the bottom?  If so, lock logic takes priority.
    local down_pos = { x = piece_pos.x, y = piece_pos.y + 1 }
    if not mino_board.test_piece(board, piece_config, down_pos, piece_rot) then
        if state.player[1].lock_tic == 0 then
            -- This is our first tic that we've locked.
            state.player[1].lock_tic = gametic
            mino_audio.playsound("step")
        end

        if gametic - state.player[1].lock_tic >= DEFAULT_LOCK_DELAY then
            -- Our lock timer has run out, lock the piece.
            mino_board.lock_piece(board, piece_config, piece_pos, piece_rot)
            mino_audio.playsound("lock")

            -- Clear the board of any lines.
            local lines = mino_board.clear_lines(board)

            -- Advance the new piece.
            if not board_next_piece(board, gametic) then
                return STATE_RESULT_GAMEOVER
            end
            mino_audio.playsound("piece0")

            -- We're done with locking, so cancel the tic out.
            state.player[1].lock_tic = 0

            -- Get the piece pointer again, because we mutated it.
            piece = mino_board.get_piece(board)
            piece_pos = mino_piece.get_pos(piece)
            piece_rot = mino_piece.get_rot(piece)
            piece_config = mino_piece.get_config(piece)
        end
    else
        -- We are not in lock logic anymore.
        state.player[1].lock_tic = 0
    end

    -- Determine what our gravity is going to be.
    local gravity_tics = 64 -- number of tics between gravity tics
    local gravity_cells = 1 -- number of cells to move the piece per gravity tics.

    -- Soft dropping and hard dropping aren't anything too special, they
    -- just toy with gravity.
    if events & EVENT_SOFTDROP ~= 0 then
        gravity_tics = 2
        gravity_cells = 1
    end

    -- If you press soft and hard drop at the same time, hard drop wins.
    -- If you hold hard drop and press soft drop afterwards, soft drop wins.
    if events & EVENT_HARDDROP ~= 0 then
        if state.player[1].harddrop_tic == 0 then
            -- We only pay attention to hard drops on the tic they were invoked.
            -- Othewise, you have rapid-fire dropping or even pieces running
            -- into each other at the top of the well.
            gravity_tics = 1
            gravity_cells = 20

            state.player[1].harddrop_tic = gametic
        end
    else
        state.player[1].harddrop_tic = 0
    end

    -- Handle gravity.
    if (gametic - state.board[1].spawn_tic) % gravity_tics >= gravity_tics - 1 then
        local src = mino_piece.get_pos(piece)
        local dst = mino_piece.get_pos(piece)
        dst.y = dst.y + gravity_cells
        local res = mino_board.test_piece_between(board, piece_config, src, piece_rot, dst)

        -- Our new location is always wherever the test tells us.  If we
        -- can't move down, we're relying on our lock delay logic to handle
        -- things next tic.
        mino_piece.set_pos(piece, res)
        piece_pos = mino_piece.get_pos(piece)

        if state.player[1].harddrop_tic == gametic then
            -- ...unless our gravity was actually a hard drop.  In that case,
            -- lock the piece immediately
            mino_board.lock_piece(board, piece_config, piece_pos, piece_rot)
            mino_audio.playsound("lock")

            -- Clear the board of any lines.
            local lines = mino_board.clear_lines(board)

            -- There is no possible other move we can make this tic.  We
            -- delete the piece here, but we don't advance to the next piece
            -- until the next tic, to ensure gravity isn't screwed up.
            mino_board.delete_piece(board)

            -- Again, doing a hard drop is mutually exclusive with any other
            -- piece movement this tic.
            return STATE_RESULT_OK
        end
    end

    -- Handle movement.
    --
    -- Here we track the number of frames we've been holding a particular
    -- direction.  We use this to track DAS and to also ensure that pressing
    -- both directions at once in a staggered way behaves correctly.
    if events & EVENT_LEFT ~= 0 then
        if state.player[1].left_tic == 0 then
            state.player[1].left_tic = gametic
        end
    else
        state.player[1].left_tic = 0
    end
    if events & EVENT_RIGHT ~= 0 then
        if state.player[1].right_tic == 0 then
            state.player[1].right_tic = gametic
        end
    else
        state.player[1].right_tic = 0
    end

    local dx = 0
    if state.player[1].left_tic == state.player[1].right_tic then
        -- Either neither event is happening, or both events started at once.
    elseif state.player[1].left_tic > state.player[1].right_tic then
        -- Ignore right event, figure out our left event DAS.
        local tics = gametic - state.player[1].left_tic
        if tics == 0 then
            -- Move immediately
            dx = -1
        elseif tics >= DEFAULT_DAS then
            -- Waited out the delay.
            tics = tics - DEFAULT_DAS
            if DEFAULT_DAS_PERIOD == 0 then
                dx = -(state.boards[0].config.width)
            elseif tics % DEFAULT_DAS_PERIOD == 0 then
                dx = -1
            end
        end
    elseif state.player[1].left_tic < state.player[1].right_tic then
        -- Ignore left event, figure out our right event DAS.
        local tics = gametic - state.player[1].right_tic
        if tics == 0 then
            -- Move immediately.
            dx = 1
        elseif tics >= DEFAULT_DAS then
            -- Waited out the delay.
            tics = tics - DEFAULT_DAS
            if DEFAULT_DAS_PERIOD == 0 then
                dx = state.boards[0].config.width
            elseif tics % DEFAULT_DAS_PERIOD == 0 then
                dx = 1
            end
        end
    end

    -- dx will be != depending on where the piece must be moved.
    if dx ~= 0 then
        local dpos = mino_piece.get_pos(piece)
        dpos.x = dpos.x + dx
        if mino_board.test_piece(board, piece_config, dpos, piece_rot) then
            mino_piece.set_pos(piece, dpos)
            piece_pos = mino_piece.get_pos(piece)
            mino_audio.playsound("move")

            -- Moving the piece successfully resets our lock timer.
            if state.player[1].lock_tic ~= 0 then
                state.player[1].lock_tic = gametic
                mino_audio.playsound("step")
            end
        end
    end

    -- Handle rotation.
    local drot = 0
    if events & EVENT_CCW ~= 0 then
        if state.player[1].ccw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot - 1
            state.player[1].ccw_already = true
        end
    else
        state.player[1].ccw_already = false
    end
    if events & EVENT_CW ~= 0 then
        if state.player[1].cw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot + 1
            state.player[1].cw_already = true
        end
    else
        state.player[1].cw_already = false
    end
    if events & EVENT_180 ~= 0 then
        drot = drot - 2
    end

    if drot ~= 0 then
        local piece_rot_count = mino_piece.config_get_rot_count(piece_config)

        local prot = piece_rot + drot
        if prot < 0 then
            prot = prot + piece_rot_count
        end
        prot = prot % piece_rot_count

        -- Figure out which wallkicks we need to calculate.
        local tries = {
            { x = 0, y = 0 },
            { x = 0, y = 0 },
            { x = 0, y = 0 },
            { x = 0, y = 0 },
            { x = 0, y = 0 },
        }

        local piece_name = mino_piece.config_get_name(piece_config)
        if piece_name == "O" then
            -- Don't wallkick the "O" piece.
        elseif piece_name == "I" then
            -- Wallkicks for the "I" piece are unique.
            if (piece_rot == ROT_0 and prot == ROT_R) or (piece_rot == ROT_L and prot == ROT_2) then
                tries[2].x = -2; tries[2].y =  0
                tries[3].x =  1; tries[3].y =  0
                tries[4].x = -2; tries[4].y =  1
                tries[5].x =  1; tries[5].y = -2
            elseif (piece_rot == ROT_R and prot == ROT_0) or (piece_rot == ROT_2 and prot == ROT_L) then
                tries[2].x =  2; tries[2].y =  0
                tries[3].x = -1; tries[3].y =  0
                tries[4].x =  2; tries[4].y = -1
                tries[5].x = -1; tries[5].y =  2
            elseif (piece_rot == ROT_0 and prot == ROT_L) or (piece_rot == ROT_R and prot == ROT_2) then
                tries[2].x = -1; tries[2].y =  0
                tries[3].x =  2; tries[3].y =  0
                tries[4].x = -1; tries[4].y = -2
                tries[5].x =  2; tries[5].y =  1
            else -- ROT_L -> ROT_0, ROT_2 -> ROT_R
                tries[2].x =  1; tries[2].y =  0
                tries[3].x = -2; tries[3].y =  0
                tries[4].x =  1; tries[4].y =  2
                tries[5].x = -2; tries[5].y = -1
            end
        else
            -- Wallkicks for the other pieces.
            if (piece_rot == ROT_0 and prot == ROT_R) or (piece_rot == ROT_2 and prot == ROT_R) then
                tries[2].x = -1; tries[2].y =  0
                tries[3].x = -1; tries[3].y = -1
                tries[4].x =  0; tries[4].y =  2
                tries[5].x = -1; tries[5].y =  2
            elseif (piece_rot == ROT_R and prot == ROT_0) or (piece_rot == ROT_R and prot == ROT_2) then
                tries[2].x =  1; tries[2].y =  0
                tries[3].x =  1; tries[3].y =  1
                tries[4].x =  0; tries[4].y = -2
                tries[5].x =  1; tries[5].y = -2
            elseif (piece_rot == ROT_0 and prot == ROT_L) or (piece_rot == ROT_2 and prot == ROT_L) then
                tries[2].x =  1; tries[2].y =  0
                tries[3].x =  1; tries[3].y = -1
                tries[4].x =  0; tries[4].y =  2
                tries[5].x =  1; tries[5].y =  2
            else -- ROT_L -> ROT_0, ROT_L -> ROT_2
                tries[2].x = -1; tries[2].y =  0
                tries[3].x = -1; tries[3].y =  1
                tries[4].x =  0; tries[4].y = -2
                tries[5].x = -1; tries[5].y = -2
            end
        end

        -- Finally, run our tests.
        for i, v in ipairs(tries) do
            local test_pos = { 
                x = piece_pos.x + v.x,
                y = piece_pos.y + v.y,
            }

            if mino_board.test_piece(board, piece_config, test_pos, prot) then
                mino_piece.set_pos(piece, test_pos)
                mino_piece.set_rot(piece, prot)
                mino_audio.playsound("rotate")

                -- Rotating the piece successfully resets our lock timer.
                if state.player[1].lock_tic ~= 0 then
                    state.player[1].lock_tic = gametic
                    mino_audio.playsound("step")
                end

                -- We don't need to test any more piece positions.
                break
            end
        end
    end

    -- if board.ghost == nil then
    --     -- Create a ghost piece.
    --     board.ghost = piece_new(piece.config)
    -- elseif board.ghost.config ~= board.piece.config then
    --     -- Update the ghost piece configuration.
    --     board.ghost.config = board.piece.config
    -- end

    -- local ghost_src = board.piece.pos
    -- local ghost_dst = { board.piece.pos.x, board.config.height }
    -- local ghost_loc = board_test_piece_between(board, board.ghost.config,
    --     ghost_src, piece.rot, ghost_dst)

    -- board.ghost.pos = ghost_loc
    -- board.ghost.rot = piece.rot

    -- We're done with all processing for this tic.
    return STATE_RESULT_OK
end

-- Bag randomizer
local function next_piece(board_id)
    local board = state.board[1]

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

return {
    state = state,
    state_frame = state_frame,
    pieces = {
        j_piece, l_piece, s_piece, z_piece, t_piece, i_piece, o_piece,
    },
    next_piece = next_piece,
}
