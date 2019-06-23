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

#include "lua.hpp"

#include "audio.hpp"

static int audioscript_playsound(lua_State* L) {
    // Parameter 1: Sound name
    const char* sound = luaL_checkstring(L, 1);

    audio_playsound(sound);
    return 0;
}

int audioscript_openlib(lua_State* L) {
    static const luaL_Reg audiolib[] = {
        { "playsound", audioscript_playsound },
        { NULL, NULL }
    };

    luaL_newlib(L, audiolib);
    return 1;
}
