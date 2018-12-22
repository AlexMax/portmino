add_subdirectory(libsamplerate-master)
set_target_properties(
    callback_hang_test
    callback_test
    clone_test
    downsample_test
    float_short_test
    misc_test
    multi_channel_test
    multichan_throughput_test
    nullptr_test
    reset_test
    simple_test
    snr_bw_test
    streaming_test
    termination_test
    throughput_test
    timewarp-file
    varispeed-play
    varispeed_test
    PROPERTIES
    EXCLUDE_FROM_ALL 1
    EXCLUDE_FROM_DEFAULT_BUILD 1
    FOLDER ExcludedTargets)

# Lua
set(LUA_DIR "lua-5.3.5/src")
set(LUA_SOURCES
    lapi.c      lapi.h
    lauxlib.c   lauxlib.h
    lbaselib.c
    lbitlib.c
    lcode.c     lcode.h
    lcorolib.c
    lctype.c    lctype.h
    ldblib.c
    ldebug.c    ldebug.h
    ldo.c       ldo.h
    ldump.c
    lfunc.c     lfunc.h
    lgc.c       lgc.h
    linit.c
    liolib.c
    llex.c      llex.h
                llimits.h
    lmathlib.c
    lmem.c      lmem.h
    loadlib.c
    lobject.c   lobject.h
    lopcodes.c  lopcodes.h
    loslib.c
    lparser.c   lparser.h
                lprefix.h
    lstate.c    lstate.h
    lstring.c   lstring.h
    lstrlib.c
    ltable.c    ltable.h
    ltablib.c
    ltm.c       ltm.h
                lua.h
                luaconf.h
                lualib.h
    lundump.c   lundump.h
    lutf8lib.c
    lvm.c       lvm.h
    lzio.c      lzio.h)

set(LUA_DIR_SOURCES "")
foreach(SOURCE ${LUA_SOURCES})
    list(APPEND LUA_DIR_SOURCES "${LUA_DIR}/${SOURCE}")
endforeach()

add_library(lua STATIC ${LUA_DIR_SOURCES})
target_include_directories(lua PUBLIC ${LUA_DIR})
