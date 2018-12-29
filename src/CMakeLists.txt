include(CheckCCompilerFlag)

function(checked_add_compile_option FLAG VAR)
    check_c_compiler_flag(${FLAG} ${VAR})
    if(${VAR})
        add_compile_options(${FLAG})
    endif()
endfunction()

if(NOT MSVC)
    add_compile_options(-Wall -Wextra)

    # Not sure if these warnings exist or not.
    checked_add_compile_option(-Werror=implicit-function-declaration E_IMPLICIT_FUNC)
    checked_add_compile_option(-Wformat=2 W_FORMAT_2)
    checked_add_compile_option(-Wnull-dereference W_NULL_DEREF)
    checked_add_compile_option(-Wrestrict W_RESTRICT)
endif()

set(GAME_SOURCES
    audio.c         audio.h
    board.c         board.h
                    define.h
                    event.h
    frontend.c      frontend.h
    game.c          game.h
    picture.c       picture.h
    piece.c         piece.h
    platform.c      platform.h
    random.c        random.h
    render.c        render.h
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
target_include_directories(portmino-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(portmino-core portmino-lib lua)
if(UNIX)
    target_link_libraries(portmino-core m)
endif()

add_subdirectory(lib)
add_subdirectory(libretro)
add_subdirectory(sdl)
