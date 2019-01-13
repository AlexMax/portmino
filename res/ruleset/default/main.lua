local function state_frame(state, playerevents)
    local events = playerevents.events[0]
    local board = state.boards[0]

    -- Get the next piece if we don't have one at this point.
    if board.piece == nil then
        if not board_next_piece(board, state.tic) then
            return STATE_RESULT_GAMEOVER
        end
    end

    local piece = board.piece

    -- Is our piece blocked from the bottom?  If so, lock logic takes priority.
    if not board_test_piece(board, piece.config, vec2i(piece.pos.x, piece.pos.y + 1), piece.rot) then
        if state.playstates[0].lock_tic == 0 then
            -- This is our first tic that we've locked.
            state.playstates[0].lock_tic = state.tic
            audio_playsound(g_sound_step)
        end

        if state.tic - state.playstates[0].lock_tic >= DEFAULT_LOCK_DELAY then
            -- Our lock timer has run out, lock the piece.
            board_lock_piece(board, piece.config, piece.pos, piece.rot)
            audio_playsound(g_sound_lock)

            -- Clear the board of any lines.
            local lines = board_clear_lines(board)

            -- Advance the new piece.
            if not board_next_piece(board, state.tic) then
                return STATE_RESULT_GAMEOVER
            end
            audio_playsound(g_sound_piece0)

            -- We're done with locking, so cancel the tic out.
            state.playstates[0].lock_tic = 0

            -- Get the piece pointer again, because we mutated it.
            piece = board.piece
        end
    else
        -- We are not in lock logic anymore.
        state.playstates[0].lock_tic = 0
    end

    -- Determine what our gravity is going to be.
    local gravity_tics = 64 -- number of tics between gravity tics
    local gravity_cells = 1 -- number of cells to move the piece per gravity tics.

    -- Soft dropping and hard dropping aren't anything too special, they
    -- just toy with gravity.
    if events & EVENT_SOFTDROP then
        gravity_tics = 2
        gravity_cells = 1
    end

    -- If you press soft and hard drop at the same time, hard drop wins.
    -- If you hold hard drop and press soft drop afterwards, soft drop wins.
    if events & EVENT_HARDDROP then
        if state.playstates[0].harddrop_tic == 0 then
            -- We only pay attention to hard drops on the tic they were invoked.
            -- Othewise, you have rapid-fire dropping or even pieces running
            -- into each other at the top of the well.
            gravity_tics = 1
            gravity_cells = 20

            state.playstates[0].harddrop_tic = state.tic
        end
    else
        state.playstates[0].harddrop_tic = 0
    end

    -- Handle gravity.
    if (state.tic - board.spawn_tic) % gravity_tics >= gravity_tics - 1 then
        local src = { piece.pos.x, piece.pos.y }
        local dst = { piece.pos.x, piece.pos.y + gravity_cells }
        local res = board_test_piece_between(board, piece.config, src, piece.rot, dst)

        -- Our new location is always wherever the test tells us.  If we
        -- can't move down, we're relying on our lock delay logic to handle
        -- things next tic.
        piece.pos.y = res.y

        if state.playstates[0].harddrop_tic == state.tic then
            -- ...unless our gravity was actually a hard drop.  In that case,
            -- lock the piece immediately
            board_lock_piece(board, piece.config, piece.pos, piece.rot)
            audio_playsound(g_sound_lock)

            -- Clear the board of any lines.
            local lines = board_clear_lines(board)

            -- There is no possible other move we can make this tic.  We
            -- delete the piece here, but we don't advance to the next piece
            -- until the next tic, to ensure gravity isn't screwed up.
            piece_delete(board.piece)
            board.piece = NULL

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
    if events & EVENT_LEFT then
        if state.playstates[0].left_tic == 0 then
            state.playstates[0].left_tic = state.tic
        end
    else
        state.playstates[0].left_tic = 0
    end
    if events & EVENT_RIGHT then
        if state.playstates[0].right_tic == 0 then
            state.playstates[0].right_tic = state.tic
        end
    else
        state.playstates[0].right_tic = 0
    end

    local dx = 0
    if state.playstates[0].left_tic == state.playstates[0].right_tic then
        -- Either neither event is happening, or both events started at once.
    elseif state.playstates[0].left_tic > state.playstates[0].right_tic then
        -- Ignore right event, figure out our left event DAS.
        local tics = state.tic - state.playstates[0].left_tic
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
    elseif state.playstates[0].left_tic < state.playstates[0].right_tic then
        -- Ignore left event, figure out our right event DAS.
        local tics = state.tic - state.playstates[0].right_tic
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
        local dpos = piece.pos
        dpos.x = dpos.x + dx
        if board_test_piece(board, piece.config, dpos, piece.rot) then
            piece.pos.x = piece.pos.x + dx
            audio_playsound(g_sound_move)

            -- Moving the piece successfully resets our lock timer.
            if state.playstates[0].lock_tic ~= 0 then
                state.playstates[0].lock_tic = state.tic
                audio_playsound(g_sound_step)
            end
        end
    end

    -- Handle rotation.
    local drot = 0
    if events & EVENT_CCW then
        if state.playstates[0].ccw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot - 1
            state.playstates[0].ccw_already = true
        end
    else
        state.playstates[0].ccw_already = false
    end
    if events & EVENT_CW then
        if state.playstates[0].cw_already == false then
            -- Only rotate if this is the first tic of the event
            drot = drot + 1
            state.playstates[0].cw_already = true
        end
    else
        state.playstates[0].cw_already = false
    end
    if events & EVENT_180 then
        drot = drot - 2
    end

    if drot ~= 0 then
        local prot = (piece.rot + drot)
        if prot < 0 then
            prot = prot + piece.config.data_count
        end
        prot = prot % piece.config.data_count

        -- Figure out which wallkicks we need to calculate.
        local tries = {
            { x = 0, y = 0 }
        }

        -- TODO: Wallkick tables go here
        -- if (piece->config == &g_o_piece) {
        --     // Don't wallkick the "O" piece.
        -- } else if (piece->config == &g_i_piece) {
        --     // Wallkicks for the "I" piece are unique.
        --     if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_L && prot == ROT_2)) {
        --         tries[1].x = -2; tries[1].y =  0;
        --         tries[2].x =  1; tries[2].y =  0;
        --         tries[3].x = -2; tries[3].y =  1;
        --         tries[4].x =  1; tries[4].y = -2;
        --     } else if ((piece->rot == ROT_R && prot == ROT_0) || (piece->rot == ROT_2 && prot == ROT_L)) {
        --         tries[1].x =  2; tries[1].y =  0;
        --         tries[2].x = -1; tries[2].y =  0;
        --         tries[3].x =  2; tries[3].y = -1;
        --         tries[4].x = -1; tries[4].y =  2;
        --     } else if ((piece->rot == ROT_0 && prot == ROT_L) || (piece->rot == ROT_R && prot == ROT_2)) {
        --         tries[1].x = -1; tries[1].y =  0;
        --         tries[2].x =  2; tries[2].y =  0;
        --         tries[3].x = -1; tries[3].y = -2;
        --         tries[4].x =  2; tries[4].y =  1;
        --     } else { /* ROT_L -> ROT_0, ROT_2 -> ROT_R */
        --         tries[1].x =  1; tries[1].y =  0;
        --         tries[2].x = -2; tries[2].y =  0;
        --         tries[3].x =  1; tries[3].y =  2;
        --         tries[4].x = -2; tries[4].y = -1;
        --     }
        -- } else {
        --     // Wallkicks for the other pieces.
        --     if ((piece->rot == ROT_0 && prot == ROT_R) || (piece->rot == ROT_2 && prot == ROT_R)) {
        --         tries[1].x = -1; tries[1].y =  0;
        --         tries[2].x = -1; tries[2].y = -1;
        --         tries[3].x =  0; tries[3].y =  2;
        --         tries[4].x = -1; tries[4].y =  2;
        --     } else if ((piece->rot == ROT_R && prot == ROT_0) || (piece->rot == ROT_R && prot == ROT_2)) {
        --         tries[1].x =  1; tries[1].y =  0;
        --         tries[2].x =  1; tries[2].y =  1;
        --         tries[3].x =  0; tries[3].y = -2;
        --         tries[4].x =  1; tries[4].y = -2;
        --     } else if ((piece->rot == ROT_0 && prot == ROT_L) || (piece->rot == ROT_2 && prot == ROT_L)) {
        --         tries[1].x = -1; tries[1].y =  0;
        --         tries[2].x = -1; tries[2].y =  1;
        --         tries[3].x =  0; tries[3].y = -2;
        --         tries[4].x = -1; tries[4].y = -2;
        --     } else /* ROT_L -> ROT_0, ROT_L -> ROT_2 */ {
        --         tries[1].x =  1; tries[1].y =  0;
        --         tries[2].x =  1; tries[2].y = -1;
        --         tries[3].x =  0; tries[3].y =  2;
        --         tries[4].x =  1; tries[4].y =  2;
        --     }
        -- }

        -- Finally, run our tests.
        for i, v in ipairs(tries) do
            local test_pos = { 
                piece.pos.x + tries[i].x,
                piece.pos.y + tries[i].y
            }
            if board_test_piece(board, piece.config, test_pos, prot) then
                piece.pos.x = piece.pos.x + tries[i].x
                piece.pos.y = piece.pos.y + tries[i].y
                piece.rot = prot
                audio_playsound(g_sound_rotate)

                -- Rotating the piece successfully resets our lock timer.
                if state.playstates[0].lock_tic ~= 0 then
                    state.playstates[0].lock_tic = state.tic
                    audio_playsound(g_sound_step)
                end

                break
            end
        end
    end


    if board.ghost == nil then
        -- Create a ghost piece.
        board.ghost = piece_new(piece.config)
    elseif board.ghost.config ~= board.piece.config then
        -- Update the ghost piece configuration.
        board.ghost.config = board.piece.config
    end

    local ghost_src = board.piece.pos
    local ghost_dst = { board.piece.pos.x, board.config.height }
    local ghost_loc = board_test_piece_between(board, board.ghost.config,
        ghost_src, piece.rot, ghost_dst)

    board.ghost.pos = ghost_loc
    board.ghost.rot = piece.rot

    -- We're done with all processing for this tic.
    return STATE_RESULT_OK
end

return {
    state_frame = state_frame
}
