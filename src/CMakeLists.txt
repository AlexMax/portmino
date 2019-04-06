include(CheckCCompilerFlag)

function(checked_add_compile_option FLAG VAR)
    check_c_compiler_flag(${FLAG} ${VAR})
    if(${VAR})
        add_compile_options(${FLAG})
    endif()
endfunction()

if(MSVC)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)

    # Extra warnings for clang-cl.exe
    checked_add_compile_option(-Wno-pragma-pack W_NO_PACK)
else()
    add_compile_options(-Wall -Wextra)

    # Extra warnings for GCC
    checked_add_compile_option(-Wrestrict W_RESTRICT)
endif()

# These warnings exist in both GCC and Clang
checked_add_compile_option(-Werror=implicit-function-declaration E_IMPLICIT_FUNC)
checked_add_compile_option(-Werror=incompatible-pointer-types E_INCOMPAT_PTR)
checked_add_compile_option(-Werror=int-conversion E_INT_CONV)
checked_add_compile_option(-Wformat=2 W_FORMAT_2)
checked_add_compile_option(-Wnull-dereference W_NULL_DEREF)

set(GAME_SOURCES
    audio.c             audio.h
    audioscript.c       audioscript.h
    board.c             board.h
    boardscript.c       boardscript.h
    define.c            define.h
    environment.c       environment.h
    error.c             error.h
    frontend.c          frontend.h
    gametype.c          gametype.h
    globalscript.c      globalscript.h
    hmap.c              hmap.h
    game.c              game.h
    ingame.c            ingame.h
    input.c             input.h
    inputscript.c       inputscript.h
    mainmenu.c          mainmenu.h
    menu.c              menu.h
    picture.c           picture.h
    piece.c             piece.h
    piecescript.c       piecescript.h
    proto.c             proto.h
    protoscript.c       protoscript.h
    platform.c          platform.h
    playmenu.c          playmenu.h
    random.c            random.h
    randomscript.c      randomscript.h
    render.c            render.h
    renderscript.c      renderscript.h
    ruleset.c           ruleset.h
    rulesetmenu.c       rulesetmenu.h
    screen.c            screen.h
    script.c            script.h
    softblock.c         softblock.h
    softfont.c          softfont.h
    softrender.c        softrender.h
    sound.c             sound.h
    vfs.c               vfs.h)

if(EMSCRIPTEN)
    set(PLATFORM_SOURCES platform_emscripten.c)
elseif(UNIX)
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
                      lua-x physfs-x)
if(UNIX)
    target_link_libraries(portmino-core m)
endif()

foreach(DIR basemino compat lib)
    add_subdirectory(${DIR})
endforeach()

if(EMSCRIPTEN)
    # Emscripten comes with SDL2 and has no concept of shared libraries.
    add_subdirectory(sdl)
else()
    add_subdirectory(libretro)
    if(SDL2_FOUND)
        add_subdirectory(sdl)
    endif()
endif()
