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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "libretro.h"

static retro_environment_t environ_cb;
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static void fallback_log(enum retro_log_level level, const char *fmt, ...) {
    (void)level;

    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
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

}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {

}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
    input_poll_cb = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
    input_state_cb = cb;
}

RETRO_API void retro_init(void) {

}

RETRO_API void retro_deinit(void) {

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
    info->geometry.base_width = 320;
    info->geometry.base_height = 240;
    info->geometry.max_width = 320;
    info->geometry.max_height = 240;
    info->geometry.aspect_ratio = 0.0;
    info->timing.fps = 60;
    info->timing.sample_rate = 44100;

    int fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device) {

}

RETRO_API void retro_reset(void) {

}

RETRO_API void retro_run(void) {
    static uint8_t buffer[320 * 240 * 4];
    memset(&buffer, 0, sizeof(buffer));

    for (int i = 0,j = 0;i < sizeof(buffer);i += 4,j += 1) {
        buffer[i] = j % 256;
        buffer[i + 1] = j % 256;
        buffer[i + 2] = j % 256;
        buffer[i + 3] = 0;
    }

    video_cb(&buffer, 320, 240, 0);
}

RETRO_API size_t retro_serialize_size(void) {
    return 0;
}

RETRO_API bool retro_serialize(void *data, size_t size) {
    return false;
}

RETRO_API bool retro_unserialize(const void *data, size_t size) {
    return false;
}

RETRO_API void retro_cheat_reset(void) {

}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) {

}

RETRO_API bool retro_load_game(const struct retro_game_info *game) {
    return true;
}

RETRO_API bool retro_load_game_special(
    unsigned game_type,
    const struct retro_game_info *info, size_t num_info
) {
    return false;
}

RETRO_API void retro_unload_game(void) {

}

RETRO_API unsigned retro_get_region(void) {
    return RETRO_REGION_NTSC;
}

RETRO_API void *retro_get_memory_data(unsigned id) {
    return NULL;
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
    return 0;
}
