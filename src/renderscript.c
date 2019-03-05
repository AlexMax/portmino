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

#include "renderscript.h"

#include "lua.h"
#include "lauxlib.h"

#include "render.h"
#include "script.h"

/**
 * Lua: Draw background
 */
static int renderscript_draw_background(lua_State* L) {
    (void)L;

    render()->draw_background();
    return 0;
}

/**
 * Lua: Draw board
 */
static int renderscript_draw_board(lua_State* L) {
    // Parameter 1: Position of the board
    vec2i_t pos;
    bool ok = script_to_vector(L, 1, &pos);
    luaL_argcheck(L, ok, 1, "invalid position");

    // Parameter 2: Board handle
    int type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid board handle");
    board_t* board = lua_touserdata(L, 2);
    if (board == NULL) {
        luaL_argerror(L, 2, "nil board handle");
        return 0;
    }

    render()->draw_board(pos, board);
    return 0;
}

/**
 * Lua: Draw text
 */
static int renderscript_draw_font(lua_State* L) {
    // Parameter 1: Position of the text
    vec2i_t pos;
    bool ok = script_to_vector(L, 1, &pos);
    luaL_argcheck(L, ok, 1, "invalid position");

    // Parameter 2: Text to render
    const char* text = luaL_checkstring(L, 2);

    render()->draw_font(pos, text);
    return 0;
}

/**
 * Lua: Draw piece
 */
static int renderscript_draw_piece(lua_State* L) {
    // Parameter 1: Position of the piece
    vec2i_t pos;
    bool ok = script_to_vector(L, 1, &pos);
    luaL_argcheck(L, ok, 1, "invalid position");

    // Parameter 2: Piece handle
    int type = lua_type(L, 2);
    luaL_argcheck(L, (type == LUA_TLIGHTUSERDATA), 2, "invalid piece handle");
    piece_config_t* piece = lua_touserdata(L, 2);
    if (piece == NULL) {
        luaL_argerror(L, 2, "nil piece handle");
        return 0;
    }

    render()->draw_piece(pos, piece);
    return 0;
}

/**
 * Push library functions into the state.
 */
int renderscript_openlib(lua_State* L) {
    static const luaL_Reg renderlib[] = {
        { "draw_background", renderscript_draw_background, },
        { "draw_board", renderscript_draw_board, },
        { "draw_font", renderscript_draw_font, },
        { "draw_piece", renderscript_draw_piece, },
        { NULL, NULL }
    };

    luaL_newlib(L, renderlib);

    return 1;
}
