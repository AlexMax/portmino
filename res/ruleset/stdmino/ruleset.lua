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

local next_buffer = require('next_buffer')
local gametype = require('gametype')

-- Stubbing out defines
local STATE_RESULT_OK = 0
local STATE_RESULT_ERROR = 1
local STATE_RESULT_GAMEOVER = 2
local STATE_RESULT_SUCCESS = 3

local DEFAULT_DAS = 12
local DEFAULT_DAS_PERIOD = 2
local DEFAULT_LOCK_DELAY = 30

local ROT_0 = 0
local ROT_R = 1
local ROT_2 = 2
local ROT_L = 3

local BOARD_PIECE = 1
local BOARD_GHOST = 2

-- The basic seven pieces we always should have access to
pieces = {
    'j_piece', 'l_piece', 's_piece', 'z_piece', 't_piece', 'i_piece', 'o_piece'
}

-- Run this on game start
local function start(state)
    -- Load the seven basic pieces
    local pieces_cfg = doconfig('pieces')
    for _, value in ipairs(pieces) do
        mino_proto.load('piece', value, pieces_cfg[value])
    end

    -- Player state
    state.player = {
        {
            -- Current score of the player.
            score = 0,

            -- Number of lines that have cleared.
            lines = 0,

            -- How many lines have been cleared in a row.
            combo = 0,

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
    }

    -- Board state
    state.board = {
        {
            -- The actual board.
            board = mino_board.new(),

            -- The "next piece" buffer.
            next = {},

            -- Tic that the current piece spawned on.
            spawn_tic = 0,

            -- Random number generator for this board.
            random = mino_random.new(nil),

            -- Bag of random pieces.
            bag = {},

            -- Bag size
            bag_size = 0,

            -- Hold piece
            hold = nil,

            -- Hold lockout, usually when somebody has already held a piece
            hold_lock = false,
        }
    }

    -- Ensure the next piece buffer is filled
    next_buffer.init_next(state.board[1])
end

-- Given a specific board, cycle to the next piece
--
-- The `force_piece` parameter is optional, set it to force a specific next
-- piece and bypass the next buffer.
local function board_next_piece(board, tic, force_piece)
    local next = board.next[1]

    if board.board:get_piece(BOARD_PIECE) ~= nil then
        board.board:unset_piece(BOARD_PIECE)
    end

    if force_piece ~= nil then
        -- Our next piece is forced.
        piece = force_piece
    else
        -- Get the next piece from the next buffer.
        piece = next_buffer.peek_next(board)
    end

    -- See if our newly-spawned piece would collide with an existing piece.
    local config = piece:config_name()
    local spawn_pos = piece:config_spawn_pos()
    local spawn_rot = piece:config_spawn_rot()
    if not board.board:test_piece(config, spawn_pos, spawn_rot) then
        spawn_pos.y = spawn_pos.y - 1
        if not board.board:test_piece(config, spawn_pos, spawn_rot) then
            return false
        end

        -- Piece spawns offset from its usual spot
        board.board:set_piece(BOARD_PIECE, piece)
        board.board:set_pos(BOARD_PIECE, spawn_pos)
    else
        board.board:set_piece(BOARD_PIECE, piece)
    end

    -- Set the spawn tic.
    board.spawn_tic = tic

    -- If we didn't force a piece, advance the next index.
    if force_piece == nil then
        next_buffer.consume_next(board)
    end

    -- Unlock the hold piece
    board.hold_lock = false

    return true
end

-- Run every frame
local function frame(state, gametic, inputs)
    local board = state.board[1]

    -- Get the next piece if we don't have one at this point.
    if board.board:get_piece(BOARD_PIECE) == nil then
        if not board_next_piece(board, gametic) then
            return STATE_RESULT_GAMEOVER
        end
        mino_audio.playsound("piece0")
    end

    -- Get our piece
    local piece = board.board:get_piece(BOARD_PIECE)
    local piece_config = piece:config_name()

    -- Handle hold piece.
    if inputs:check_hold(1) then
        if board.hold_lock == false then
            if board.hold == nil then
                -- Hold this piece
                board.hold = mino_piece.new(piece_config)

                -- We have no other held piece, so generate a new one
                if not board_next_piece(board, gametic) then
                    return STATE_RESULT_GAMEOVER
                end
            else
                -- Hold this piece
                local swapped_piece = board.hold
                board.hold = mino_piece.new(piece_config)

                -- Spawn the held piece
                if not board_next_piece(board, gametic, swapped_piece) then
                    return STATE_RESULT_GAMEOVER
                end
            end

            -- Lock out holding until the next piece
            board.hold_lock = true

            -- Ensure that our piece variables are up to date
            piece = board.board:get_piece(BOARD_PIECE)
            piece_config = piece:config_name()
        end
    end

    -- Grab the piece position and rotation
    local piece_pos = board.board:get_pos(BOARD_PIECE)
    local piece_rot = board.board:get_rot(BOARD_PIECE)

    -- Handle rotation.
    local drot = 0
    if inputs:check_ccw(1) then
        if state.player[1].ccw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot - 1
            state.player[1].ccw_already = true
        end
    else
        state.player[1].ccw_already = false
    end
    if inputs:check_cw(1) then
        if state.player[1].cw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot + 1
            state.player[1].cw_already = true
        end
    else
        state.player[1].cw_already = false
    end
    if inputs:check_180(1) then
        -- FIXME: This doesn't have debouncing yet
        drot = drot - 2
    end

    if drot ~= 0 then
        local piece = board.board:get_piece(BOARD_PIECE)
        local piece_rot_count = piece:config_rot_count()

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

        local piece_name = piece:config_name()
        if piece_name == "o_piece" then
            -- Don't wallkick the "O" piece.
        elseif piece_name == "i_piece" then
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

            if board.board:test_piece(piece_config, test_pos, prot) then
                board.board:set_pos(BOARD_PIECE, test_pos)
                board.board:set_rot(BOARD_PIECE, prot)
                mino_audio.playsound("rotate")

                -- Make sure that we update our board piece information so
                -- shifts take into account our new position.
                piece_pos = test_pos
                piece_rot = prot

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

    -- Handle movement.
    --
    -- Here we track the number of frames we've been holding a particular
    -- direction.  We use this to track DAS and to also ensure that pressing
    -- both directions at once in a staggered way behaves correctly.
    if inputs:check_left(1) then
        if state.player[1].left_tic == 0 then
            state.player[1].left_tic = gametic
        end
    else
        state.player[1].left_tic = 0
    end
    if inputs:check_right(1) then
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
        local dpos = board.board:get_pos(BOARD_PIECE)
        dpos.x = dpos.x + dx
        if board.board:test_piece(piece_config, dpos, piece_rot) then
            board.board:set_pos(BOARD_PIECE, dpos)
            piece_pos = board.board:get_pos(BOARD_PIECE)
            mino_audio.playsound("move")

            -- Moving the piece successfully resets our lock timer.
            if state.player[1].lock_tic ~= 0 then
                state.player[1].lock_tic = gametic
                mino_audio.playsound("step")
            end
        end
    end

    -- Is our piece blocked from the bottom?  If so, lock logic takes priority
    -- over gravity logic.
    local down_pos = { x = piece_pos.x, y = piece_pos.y + 1 }
    if not board.board:test_piece(piece_config, down_pos, piece_rot) then
        if state.player[1].lock_tic == 0 then
            -- This is our first tic that we've locked.
            state.player[1].lock_tic = gametic
            mino_audio.playsound("step")
        end

        if gametic - state.player[1].lock_tic >= DEFAULT_LOCK_DELAY then
            -- Our lock timer has run out, lock the piece.
            board.board:lock_piece(piece_config, piece_pos, piece_rot)
            mino_audio.playsound("lock")

            -- Clear the board of any lines.
            local lines = board.board:clear_lines()

            -- We're done with locking, so cancel the tic out.
            state.player[1].lock_tic = 0

            -- There is no possible other move we can make this tic.  We
            -- delete the piece here, but we don't advance to the next piece
            -- until the next tic, to ensure gravity isn't screwed up.
            board.board:unset_piece(BOARD_PIECE)
            board.board:unset_piece(BOARD_GHOST)

            -- Again, piece lock is mutually exclusive with any other
            -- piece movement this tic.
            return STATE_RESULT_OK
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
    if inputs:check_softdrop(1) then
        gravity_tics = 2
        gravity_cells = 1
    end

    -- If you press soft and hard drop at the same time, hard drop wins.
    -- If you hold hard drop and press soft drop afterwards, soft drop wins.
    if inputs:check_harddrop(1) then
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
        local src = board.board:get_pos(BOARD_PIECE)
        local dst = board.board:get_pos(BOARD_PIECE)
        dst.y = dst.y + gravity_cells
        local res = board.board:test_piece_between(piece_config, src, piece_rot, dst)

        -- Our new location is always wherever the test tells us.  If we
        -- can't move down, we're relying on our lock delay logic to handle
        -- things next tic.
        board.board:set_pos(BOARD_PIECE, res)
        piece_pos = board.board:get_pos(BOARD_PIECE)

        if state.player[1].harddrop_tic == gametic then
            -- ...unless our gravity was actually a hard drop.  In that case,
            -- lock the piece immediately
            board.board:lock_piece(piece_config, piece_pos, piece_rot)
            mino_audio.playsound("lock")

            -- Clear the board of any lines.
            local lines = board.board:clear_lines()

            -- There is no possible other move we can make this tic.  We
            -- delete the piece here, but we don't advance to the next piece
            -- until the next tic, to ensure gravity isn't screwed up.
            board.board:unset_piece(BOARD_PIECE)
            board.board:unset_piece(BOARD_GHOST)

            -- Again, doing a hard drop is mutually exclusive with any other
            -- piece movement this tic.
            return STATE_RESULT_OK
        end
    end

    -- Create the ghost piece
    local ghost = board.board:set_piece(BOARD_GHOST, piece)

    -- Push the ghost to the bottom of the screen
    local ghost_dst = { x = piece_pos.x, y = 22 }
    local ghost_pos = board.board:test_piece_between(piece_config,
        piece_pos, piece_rot, ghost_dst)
    board.board:set_pos(BOARD_GHOST, ghost_pos)
    board.board:set_rot(BOARD_GHOST, piece_rot)

    -- Our gametype might want to end the game, so check it first.
    -- TODO: Add this functionality

    -- We're done with this tic.
    return STATE_RESULT_OK
end

-- Run every frame to draw the game
local function draw(state)
    gametype.draw(state)
end

return {
    start = start,
    frame = frame,
    draw = draw,
}
