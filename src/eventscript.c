/**
 * This file is part of Portmino.
 * 
 * Portmino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Portmino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Portmino.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "event.h"
#include "eventscript.h"

typedef struct {
    playerevents_t* playerevents;
    size_t player;
} esca_t;

/**
 * Check the arguments for event checking functions
 * 
 * Returns the 0-indexed player index to check.
 */
static esca_t eventscript_check_args(lua_State* L) {
    esca_t ret;

    // Check internal state.
    lua_pushstring(L, "playerevents");
    if (lua_gettable(L, LUA_REGISTRYINDEX) != LUA_TLIGHTUSERDATA) {
        // never returns
        luaL_argerror(L, 1, "no events are available");
    }
    ret.playerevents = lua_touserdata(L, -1);

    // Parameter 1: Player number, 1-indexed.
    lua_Integer player = luaL_checkinteger(L, 1);
    luaL_argcheck(L, (player >= 1 && player <= MINO_MAX_PLAYERS), 1, "invalid player index");
    ret.player = player - 1;

    return ret;
}

/**
 * Check a particular player's events to see if a EVENT_LEFT is set
 */
static int eventscript_check_left(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_LEFT;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_RIGHT is set
 */
static int eventscript_check_right(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_RIGHT;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_SOFTDROP is set
 */
static int eventscript_check_softdrop(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_SOFTDROP;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_HARDDROP is set
 */
static int eventscript_check_harddrop(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_HARDDROP;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_CCW is set
 */
static int eventscript_check_ccw(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_CCW;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_CW is set
 */
static int eventscript_check_cw(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_CW;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_HOLD is set
 */
static int eventscript_check_hold(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_HOLD;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's events to see if a EVENT_180 is set
 */
static int eventscript_check_180(lua_State* L) {
    esca_t args = eventscript_check_args(L);
    int pressed = args.playerevents->events[args.player] & EVENT_180;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Initialize the events module.
 */
int eventscript_openlib(lua_State* L) {
    static const luaL_Reg eventlib[] = {
        { "check_left", eventscript_check_left },
        { "check_right", eventscript_check_right },
        { "check_softdrop", eventscript_check_softdrop },
        { "check_harddrop", eventscript_check_harddrop },
        { "check_ccw", eventscript_check_ccw },
        { "check_cw", eventscript_check_cw },
        { "check_hold", eventscript_check_hold },
        { "check_180", eventscript_check_180 },
        { NULL, NULL }
    };

    luaL_newlib(L, eventlib);
    return 1;
}
