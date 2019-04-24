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

#include "lauxlib.h"

#include "input.h"

typedef struct {
    playerinputs_t* playerinputs;
    size_t player;
} isca_t;

/**
 * Check the arguments for input checking functions
 * 
 * Returns the 0-indexed player index to check.
 */
static isca_t inputscript_check_args(lua_State* L) {
    isca_t ret;

    // Parameter 1: Our userdata
    ret.playerinputs = luaL_checkudata(L, 1, "inputs_t");

    // Parameter 2: Player number, 1-indexed.
    lua_Integer player = luaL_checkinteger(L, 2);
    luaL_argcheck(L, (player >= 1 && player <= MINO_MAX_PLAYERS), 2, "invalid player index");
    ret.player = player - 1;

    return ret;
}

/**
 * Check a particular player's inputs to see if a INPUT_LEFT is set
 */
static int inputscript_check_left(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_LEFT;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_RIGHT is set
 */
static int inputscript_check_right(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_RIGHT;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_SOFTDROP is set
 */
static int inputscript_check_softdrop(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_SOFTDROP;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_HARDDROP is set
 */
static int inputscript_check_harddrop(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_HARDDROP;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_CCW is set
 */
static int inputscript_check_ccw(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_CCW;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_CW is set
 */
static int inputscript_check_cw(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_CW;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_HOLD is set
 */
static int inputscript_check_hold(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_HOLD;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Check a particular player's inputs to see if a INPUT_180 is set
 */
static int inputscript_check_180(lua_State* L) {
    isca_t args = inputscript_check_args(L);
    int pressed = args.playerinputs->inputs[args.player] & INPUT_180;
    lua_pushboolean(L, pressed);
    return 1;
}

/**
 * Push a new inputs userdata to the stack.
 */
void inputscript_push_inputs(lua_State* L, const playerinputs_t* inputs) {
    // Copy our inputs into a new userdata
    playerinputs_t* ud = lua_newuserdata(L, sizeof(playerinputs_t));
    memcpy(ud, inputs, sizeof(*ud));

    // Apply methods to the inputs
    luaL_setmetatable(L, "inputs_t");
}

/**
 * Initialize the inputs module.
 */
int inputscript_openlib(lua_State* L) {
    static const luaL_Reg inputstype[] = {
        { "check_left", inputscript_check_left },
        { "check_right", inputscript_check_right },
        { "check_softdrop", inputscript_check_softdrop },
        { "check_harddrop", inputscript_check_harddrop },
        { "check_ccw", inputscript_check_ccw },
        { "check_cw", inputscript_check_cw },
        { "check_hold", inputscript_check_hold },
        { "check_180", inputscript_check_180 },
        { NULL, NULL }
    };

    luaL_newmetatable(L, "inputs_t");

    luaL_newlib(L, inputstype);
    lua_setfield(L, -2, "__index");

    lua_pop(L, 1);

    return 1;
}
