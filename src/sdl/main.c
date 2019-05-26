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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "audio.h"
#include "error.h"
#include "input.h"
#include "frontend.h"
#include "game.h"
#include "platform.h"
#include "softrender.h"

static SDL_Window* g_window;
static SDL_Renderer* g_renderer;
static SDL_Texture* g_texture;
static SDL_AudioDeviceID g_audio_device;

static double g_pfreq;

#if defined(MINO_EMBED_RESOURCE)

#include "basemino.h"

static buffer_t g_basemino;

static buffer_t* sdl_basemino(void) {
    if (g_basemino.data == NULL) {
        g_basemino.data = basemino_pk3;
        g_basemino.size = basemino_pk3_len;
    }
    return &g_basemino;
}

#else

static buffer_t* sdl_basemino(void) {
    return NULL;
}

#endif

ATTRIB_PRINTF(1, 0)
static void sdl_fatalerror(const char *fmt, va_list va) {
    char buffer[8192];
    vsnprintf(buffer, sizeof(buffer), fmt, va);

    // Display any error messages we have stacked up.
    char work[1024];
    if (error_count() > 0) {
        // Show a count of errors
        snprintf(work, sizeof(work), "\nMost recent %zu errors...", error_count());
        strncat(buffer, work, sizeof(buffer) - 1);

        // Pop all errors from the queue
        char* i = NULL;
        while ((i = error_pop()) != NULL) {
            strncat(buffer, "\n- ", sizeof(buffer) - 1);
            strncat(buffer, i, sizeof(buffer) - 1);
        }
    } else {
        // Why are we erroring out if the error stack is empty?
        strncat(buffer, "\nError stack is empty...", sizeof(buffer) - 1);
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Portmino", buffer, NULL);
    fputs(buffer, stderr);

    exit(1);
}

static input_t sdl_scancode_to_input(int code) {
    switch (code) {
    case SDL_SCANCODE_LEFT: return INPUT_LEFT;
    case SDL_SCANCODE_RIGHT: return INPUT_RIGHT;
    case SDL_SCANCODE_DOWN: return INPUT_SOFTDROP;
    case SDL_SCANCODE_UP: return INPUT_HARDDROP;
    case SDL_SCANCODE_Z: return INPUT_CCW;
    case SDL_SCANCODE_X: return INPUT_CW;
    case SDL_SCANCODE_C: return INPUT_HOLD;
    default: return INPUT_NONE;
    }
}

static iinput_t sdl_scancode_to_iinput(int code) {
    switch (code) {
    case SDL_SCANCODE_ESCAPE: return IINPUT_PAUSE;
    case SDL_SCANCODE_F10: return IINPUT_RESTART;
    case SDL_SCANCODE_F11: return IINPUT_MAINMENU;
    case SDL_SCANCODE_F12: return IINPUT_QUIT;
    default: return IINPUT_NONE;
    }
}

static minput_t sdl_scancode_to_minput(int code) {
    switch (code) {
    case SDL_SCANCODE_UP: return MINPUT_UP;
    case SDL_SCANCODE_DOWN: return MINPUT_DOWN;
    case SDL_SCANCODE_LEFT: return MINPUT_LEFT;
    case SDL_SCANCODE_RIGHT: return MINPUT_RIGHT;
    case SDL_SCANCODE_RETURN: return MINPUT_OK;
    case SDL_SCANCODE_BACKSPACE: return MINPUT_CANCEL;
    default: return MINPUT_NONE;
    }
}

/**
 * An SDL_AudioCallback function
 */
static void sdl_audio_cb(void* _, uint8_t* stream, int len) {
    // Play audio for the buffer
    audio_context_t* audio_ctx = audio_frame(len / sizeof(int16_t) / MINO_AUDIO_CHANNELS);
    if (audio_ctx == NULL) {
        // Audio context isn't available, play silence
        memset(stream, 0x00, len);
        return;
    }

    // Copy audio to the buffer
    memcpy(stream, audio_ctx->sampledata, audio_ctx->bytesize);
}

/**
 * Run one gametic worth of input, gameplay, and video
 */
static void sdl_run(void) {
    // Performance counter
    uint64_t pcount;

    // Assemble our game inputs from polled inputs.
    SDL_Event input;
    static gameinputs_t inputs = { 0 }; // keep the inputs around

    while (SDL_PollEvent(&input)) {
        switch (input.type) {
        case SDL_KEYDOWN:
            if (input.key.repeat != 0) {
                break;
            }
            inputs.game.inputs[0] |= sdl_scancode_to_input(input.key.keysym.scancode);
            inputs.interface.inputs[0] |= sdl_scancode_to_iinput(input.key.keysym.scancode);
            inputs.menu.inputs[0] |= sdl_scancode_to_minput(input.key.keysym.scancode);
            break;
        case SDL_KEYUP:
            if (input.key.repeat != 0) {
                break;
            }
            inputs.game.inputs[0] &= ~(sdl_scancode_to_input(input.key.keysym.scancode));
            inputs.interface.inputs[0] &= ~(sdl_scancode_to_iinput(input.key.keysym.scancode));
            inputs.menu.inputs[0] &= ~(sdl_scancode_to_minput(input.key.keysym.scancode));
            break;
        case SDL_QUIT:
            exit(0);
            break;
        }
    }

    // Run the game simulation.
    pcount = SDL_GetPerformanceCounter();
    game_frame(&inputs);
    double game_time = (SDL_GetPerformanceCounter() - pcount) / g_pfreq;

    // Render the screen.
    pcount = SDL_GetPerformanceCounter();
    softrender_context_t* context = game_draw();
    double draw_time = (SDL_GetPerformanceCounter() - pcount) / g_pfreq;

    pcount = SDL_GetPerformanceCounter();
    SDL_UpdateTexture(g_texture, NULL, context->buffer.data, context->buffer.width * MINO_SOFTRENDER_BPP);
    SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
    SDL_RenderPresent(g_renderer);
    double render_time = (SDL_GetPerformanceCounter() - pcount) / g_pfreq;

    if (false) {
        SDL_Log("game %f, draw %f, render %f\n", game_time, draw_time,
                render_time);
    }

    return;
}

/**
 * The main loop that runs the game
 *
 * We need to hit 16.666...ms as often as possible, so we can prevent clock
 * drift and stuttering sound buffers.  We start with a target frametime of
 * 17ms, 17ms, 16ms over three frames, which adds up to the same thing as
 * 16.666... over three frames.
 * 
 * In addition, we keep an "adjustment" number that is calculated by
 * comparing the time it took to run the last frame (including any sleep)
 * to our target time.  If the frame is short, we don't particularly care,
 * but if the frame goes long, we amortize the discrepency by subtracting
 * the sleep by one over the next few frames.
 */
void sdl_loop(void) {
    int targets[] = { 17, 17, 16 };
    size_t i = 0;
    int32_t fadjust = 0;
    while (true) {
        int ftarget = targets[i];
        if (fadjust < 0) {
            // Amortize the adjustment over many frames.
            ftarget -= 1;
            fadjust += 1;
        }

        // Actually run the frame, calculating how long we need to sleep.
        uint32_t fstart = SDL_GetTicks();
        sdl_run();
        int32_t ftime = SDL_GetTicks() - fstart;

        if (ftime < ftarget) {
            // Sleep the rest of the frametime.
            SDL_Delay(ftarget - ftime);
        }

        // Find any discrepencies between our desired total frame time
        // and actual frame time over the next few tics. 
        int32_t fttime = SDL_GetTicks() - fstart;
        int32_t adjust = ftarget - fttime;
        if (adjust < 0) {
            fadjust += adjust;
        }

        i = (i + 1) % 3;
    }
}

/**
 * The main atexit handler
 */
static void sdl_clean_exit(void) {
    game_deinit();

    SDL_CloseAudioDevice(g_audio_device);
    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    platform_deinit();
    frontend_deinit();
}

int main(int argc, char** argv) {
    atexit(sdl_clean_exit);

    // Initialize the front-end.
    frontend_module_t frontend = {
        sdl_basemino, sdl_fatalerror
    };
    if (!frontend_init(&frontend)) {
        fprintf(stderr, "frontend_init failure\n");
        return 1;
    }

    // Initialize our non-library platform-specific functions first.
    if (!platform_init()) {
        fprintf(stderr, "platform_init failure\n");
        return 1;
    }

    // Now initialize SDL...
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) != 0) {
        fprintf(stderr, "SDL2_Init failure:  %s\n", SDL_GetError());
        return 1;
    }

    // Calculate the performance frequency here.
    g_pfreq = SDL_GetPerformanceFrequency() / 1000;

    g_window = SDL_CreateWindow("Portmino", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if (g_window == NULL) {
        frontend_fatalerror("SDL_CreateWindow failure: %s\n", SDL_GetError());
        return 1;
    }

    // SDL_RENDERER_PRESENTVSYNC for VSync
    g_renderer = SDL_CreateRenderer(g_window, -1, 0);
    if (g_renderer == NULL) {
        frontend_fatalerror("SDL_Renderer failure: %s\n", SDL_GetError());
        return 1;
    }

    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MINO_SOFTRENDER_WIDTH, MINO_SOFTRENDER_HEIGHT);
    if (g_texture == NULL) {
        frontend_fatalerror("SDL_CreateTexture failure: %s\n", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec s_expect, s_actual;
    SDL_zero(s_expect);
    s_expect.freq = MINO_AUDIO_HZ;
    s_expect.format = AUDIO_S16;
    s_expect.channels = 2;
    s_expect.samples = 512;
    s_expect.callback = sdl_audio_cb;
    g_audio_device = SDL_OpenAudioDevice(NULL, 0, &s_expect, &s_actual, 0);
    if (g_audio_device == 0) {
        frontend_fatalerror("SDL_OpenAudioDevice failure: %s\n", SDL_GetError());
        return 1;
    }
    SDL_PauseAudioDevice(g_audio_device, 0);

    // Initialize the game before we run it.
    if (!game_init(argc, argv)) {
        frontend_fatalerror("Could not initialize the game.");
        return 1;
    }

#ifdef __EMSCRIPTEN__
    // Emscripten runs our main loop for us...
    emscripten_set_main_loop(sdl_run, 0, 1);
    return 0;
#endif

    // Run our main loop.  Never returns.
    sdl_loop();
    return 1;
}
