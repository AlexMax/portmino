include(CheckCCompilerFlag)

function(checked_add_compile_option FLAG VAR)
    check_c_compiler_flag(${FLAG} ${VAR})
    if(${VAR})
        add_compile_options(${FLAG})
    endif()
endfunction()

if(MSVC)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra)

    # Not sure if these warnings exist or not.
    checked_add_compile_option(-Werror=implicit-function-declaration E_IMPLICIT_FUNC)
    checked_add_compile_option(-Werror=incompatible-pointer-types E_INCOMPAT_PTR)
    checked_add_compile_option(-Wformat=2 W_FORMAT_2)
    checked_add_compile_option(-Wnull-dereference W_NULL_DEREF)
    checked_add_compile_option(-Wrestrict W_RESTRICT)
endif()

set(GAME_SOURCES
    audio.c         audio.h
    board.c         board.h
    boardscript.c   boardscript.h
    define.c        define.h
                    event.h
    frontend.c      frontend.h
    hmap.c          hmap.h
    game.c          game.h
    picture.c       picture.h
    piece.c         piece.h
    piecescript.c   piecescript.h
    platform.c      platform.h
    random.c        random.h
    render.c        render.h
    ruleset.c       ruleset.h
    script.c        script.h
    softblock.c     softblock.h
    softfont.c      softfont.h
    softrender.c    softrender.h
    sound.c         sound.h
    state.c         state.h
    vfs.c           vfs.h)

if(UNIX)
    set(PLATFORM_SOURCES platform_unix.c)
elseif(WIN32)
    set(PLATFORM_SOURCES platform_win32.c)
else()
    message(FATAL_ERROR "Platform implementation missing")
endif()

add_library(portmino-core STATIC ${GAME_SOURCES} ${LIBRARY_SOURCES} ${PLATFORM_SOURCES})
add_sanitizers(portmino-core)
set_property(TARGET portmino-core PROPERTY POSITION_INDEPENDENT_CODE ON)
if(UNIX)
    target_compile_definitions(portmino-core PRIVATE _GNU_SOURCE)
endif()
target_include_directories(portmino-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(portmino-core portmino-lib portmino-compat
                      lua physfs-static)
if(UNIX)
    target_link_libraries(portmino-core m)
endif()

add_subdirectory(compat)
add_subdirectory(lib)
add_subdirectory(libretro)

# Only build SDL front-end if we've found SDL2
if(SDL2_FOUND)
    add_subdirectory(sdl)
endif()
