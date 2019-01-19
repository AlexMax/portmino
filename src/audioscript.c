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

#include "audio.h"

static int audioscript_playsound(lua_State* L) {
    // Parameter 1: Sound name
    const char* sound = luaL_checkstring(L, 1);
    if (strcmp(sound, "gameover") == 0) {
        audio_playsound(g_sound_gameover);
    } else if (strcmp(sound, "lock") == 0) {
        audio_playsound(g_sound_lock);
    } else if (strcmp(sound, "move") == 0) {
        audio_playsound(g_sound_move);
    } else if (strcmp(sound, "piece0") == 0) {
        audio_playsound(g_sound_piece0);
    } else if (strcmp(sound, "rotate") == 0) {
        audio_playsound(g_sound_rotate);
    } else if (strcmp(sound, "step") == 0) {
        audio_playsound(g_sound_step);
    }

    return 0;
}

int audio_openlib(lua_State* L) {
    static const luaL_Reg audiolib[] = {
        { "playsound", audioscript_playsound },
        { NULL, NULL }
    };

    luaL_newlib(L, audiolib);
    return 1;
}
