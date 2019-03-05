-- Contains any important state that we need to keep track of.
local function init()
    local state = mino_state.get()

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
            next = mino_next.new(1),

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
end

return init
