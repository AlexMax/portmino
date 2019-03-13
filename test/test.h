#pragma once

#include "define.h"

#include "basemino.h"
#include "frontend.h"

static buffer_t g_basemino;

static buffer_t* test_basemino(void) {
    if (g_basemino.data == NULL) {
        g_basemino.data = basemino_pk3;
        g_basemino.size = basemino_pk3_len;
    }
    return &g_basemino;
}

static void test_fatalerror(const char *fmt, va_list va) {
    fail_msg("fatalerror triggered\n");
}

static frontend_module_t g_frontend_module = {
    test_basemino,
    test_fatalerror,
};
