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

#include "libretro.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "audio.hpp"
#include "basemino.h"
#include "input.hpp"
#include "frontend.hpp"
#include "game.hpp"
#include "platform.hpp"
#include "softrender.hpp"

static retro_environment_t environ_cb;
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static buffer_t g_basemino;

static buffer_t* retro_basemino(void) {
    if (g_basemino.data == NULL) {
        g_basemino.data = basemino_pk3;
        g_basemino.size = basemino_pk3_len;
    }
    return &g_basemino;
}

ATTRIB_PRINTF(1, 0)
static void retro_fatalerror(const char *fmt, va_list va) {
    char buffer[8192];
    vsnprintf(buffer, sizeof(buffer), fmt, va);
    log_cb(RETRO_LOG_ERROR, "Portmino Fatal Error: %s\n", buffer);
    exit(1);
}

ATTRIB_PRINTF(2, 3)
static void fallback_log(enum retro_log_level level, const char *fmt, ...) {
    (void)level;

    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

static inputs_t retro_input_to_input(unsigned port) {
    input_t ret = 0;
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) {
        ret |= INPUT_LEFT;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) {
        ret |= INPUT_RIGHT;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) {
        ret |= INPUT_SOFTDROP;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) {
        ret |= INPUT_HARDDROP;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)) {
        ret |= INPUT_CCW;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)) {
        ret |= INPUT_CW;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R)) {
        ret |= INPUT_HOLD;
    }
    return ret;
}

static inputs_t retro_input_to_minput(unsigned port) {
    input_t ret = 0;
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) {
        ret |= MINPUT_UP;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) {
        ret |= MINPUT_DOWN;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) {
        ret |= MINPUT_LEFT;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) {
        ret |= MINPUT_RIGHT;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)) {
        ret |= MINPUT_OK;
    }
    if (input_state_cb(port, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)) {
        ret |= MINPUT_CANCEL;
    }
    return ret;
}

RETRO_API void retro_set_environment(retro_environment_t cb) {
    environ_cb = cb;

    bool no_rom = true;
    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);

    struct retro_log_callback logging;
    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging)) {
        log_cb = logging.log;
    } else {
        log_cb = fallback_log;
    }
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) {
    video_cb = cb;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) {
    (void)cb;
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    audio_batch_cb = cb;
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
}

RETRO_API void retro_init(void) {
    frontend_module_t module = {
        retro_basemino,
        retro_fatalerror,
    };
    frontend_init(&module);
    platform_init();
    game_init(0, NULL);
}

RETRO_API void retro_deinit(void) {
    game_deinit();
    platform_deinit();
    frontend_deinit();
}

RETRO_API unsigned retro_api_version(void) {
    return RETRO_API_VERSION;
}

RETRO_API void retro_get_system_info(struct retro_system_info *info) {
    memset(info, 0, sizeof(*info));
    info->library_name = "Portmino";
    info->library_version = "0.1.0";
    info->valid_extensions = "";
    info->need_fullpath = false;
    info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info) {
    memset(info, 0, sizeof(*info));
    info->geometry.base_width = MINO_SOFTRENDER_WIDTH;
    info->geometry.base_height = MINO_SOFTRENDER_HEIGHT;
    info->geometry.max_width = MINO_SOFTRENDER_WIDTH;
    info->geometry.max_height = MINO_SOFTRENDER_HEIGHT;
    info->geometry.aspect_ratio = 0.0;
    info->timing.fps = MINO_FPS;
    info->timing.sample_rate = MINO_AUDIO_HZ;

    int fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device) {
    (void)port; (void)device;
}

RETRO_API void retro_reset(void) {
    game_deinit();
    game_init(0, NULL);
}

RETRO_API void retro_run(void) {
    input_poll_cb();

    gameinputs_t inputs = { 0 };
    inputs.game.inputs[0] = retro_input_to_input(0);
    inputs.interface.inputs[0] = 0;
    inputs.menu.inputs[0] = retro_input_to_minput(0);

    // Run the game simulation.
    game_frame(&inputs);

    // Render the screen.
    softrender_context_t* render_ctx = game_draw();
    video_cb(render_ctx->buffer.data, render_ctx->buffer.width, render_ctx->buffer.height, render_ctx->buffer.width * 4);

    // Play a tic worth of audio.
    audio_context_t* audio_ctx = audio_frame(MINO_AUDIO_HZ / MINO_FPS);
    audio_batch_cb(audio_ctx->sampledata, audio_ctx->framecount);
}

RETRO_API size_t retro_serialize_size(void) {
    return 0;
}

RETRO_API bool retro_serialize(void *data, size_t size) {
    (void)data; (void)size;
    return false;
}

RETRO_API bool retro_unserialize(const void *data, size_t size) {
    (void)data; (void)size;
    return false;
}

RETRO_API void retro_cheat_reset(void) {

}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) {
    (void)index; (void)enabled; (void)code;
}

RETRO_API bool retro_load_game(const struct retro_game_info *game) {
    (void)game;
    return true;
}

RETRO_API bool retro_load_game_special(
    unsigned game_type,
    const struct retro_game_info *info, size_t num_info
) {
    (void)game_type; (void)info; (void)num_info;
    return false;
}

RETRO_API void retro_unload_game(void) {

}

RETRO_API unsigned retro_get_region(void) {
    return RETRO_REGION_NTSC;
}

RETRO_API void *retro_get_memory_data(unsigned id) {
    (void)id;
    return NULL;
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
    (void)id;
    return 0;
}
