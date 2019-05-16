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

-- Gravity is measured in G's.  1.0G is down once a frame, 0.5G is
-- down every other frame, 2.0G is down twice a frame, et cetera.
--
-- To prevent us from having to deal with potentially non-deterministic
-- floating point math, we treat gravity values as 16.16 fixed-point
-- integers.  This essentially splits the number into two parts, the
-- whole number part (upper 16 bits) and the fractional parts (lower
-- 16 bits).  This ensures our math is consistent and that replays and
-- netplay work the same across different types of computers.
--
-- Of course, this being Lua, we can't ever guarantee that the numbers
-- we're dealing with are ever actually integers.  But as long as you
-- just stick to add, subtract, multiply by whole numbers, and use
-- integer division (the // operator), you should be good.  If you -
-- for whatever reason - need a way to multiply or divide by a fraction
-- bug the maintainer to give you a "FixedMul" and "FixedDiv" function
-- written in C.  Also, this should probably go without saying, but don't
-- use Lua standard library math functions on these, unless you want wrong
-- answers.

local GRAVITY_UNIT = 1 << 16
local GRAVITY_LEVEL = {
    1092, 1377, 1767, 2310, 3075, 4168, 5758, 8107, 11634, 17026,
    25413, 38705, 60179, 95477, 154711, 256400, 433439, 748127,
    1332032, 2374492,
}

-- Convert a player's level into an appropriate gravity value
local function player_to_gravity(player)
    local gravity = GRAVITY_LEVEL[player.level]
    if gravity ~= nil then
        return gravity
    end

    -- Max out at level 20
    return GRAVITY_LEVEL[20]
end

return {
    player_to_gravity = player_to_gravity,
    GRAVITY_UNIT = GRAVITY_UNIT,
}
