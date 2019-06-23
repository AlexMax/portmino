include(Portmino)

set(GAME_SOURCES
    audio.cpp           audio.hpp
    audioscript.cpp     audioscript.hpp
    board.cpp           board.hpp
    boardscript.cpp     boardscript.hpp
    define.cpp          define.hpp
    entity.cpp          entity.hpp
    environment.cpp     environment.hpp
    error.cpp           error.hpp
    frontend.cpp        frontend.hpp
    gametype.cpp        gametype.hpp
    globalscript.cpp    globalscript.hpp
    game.cpp            game.hpp
    ingame.cpp          ingame.hpp
    input.cpp           input.hpp
    inputscript.cpp     inputscript.hpp
    mainmenu.cpp        mainmenu.hpp
    menu.cpp            menu.hpp
    pausemenu.cpp       pausemenu.hpp
    picture.cpp         picture.hpp
    piece.cpp           piece.hpp
    piecescript.cpp     piecescript.hpp
    proto.cpp           proto.hpp
    protoscript.cpp     protoscript.hpp
    platform.cpp        platform.hpp
    playmenu.cpp        playmenu.hpp
    random.cpp          random.hpp
    randomscript.cpp    randomscript.hpp
    render.cpp          render.hpp
    renderscript.cpp    renderscript.hpp
    ruleset.cpp         ruleset.hpp
    rulesetmenu.cpp     rulesetmenu.hpp
    screen.cpp          screen.hpp
    script.cpp          script.hpp
    serialize.cpp       serialize.hpp
    softblock.cpp       softblock.hpp
    softfont.cpp        softfont.hpp
    softrender.cpp      softrender.hpp
    sound.cpp           sound.hpp
    vfs.cpp             vfs.hpp)

if(EMSCRIPTEN)
    set(PLATFORM_SOURCES platform_emscripten.cpp)
elseif(UNIX)
    set(PLATFORM_SOURCES platform_unix.cpp)
elseif(WIN32)
    set(PLATFORM_SOURCES platform_win32.cpp)
else()
    message(FATAL_ERROR "Platform implementation missing")
endif()

add_library(portmino-core STATIC ${GAME_SOURCES} ${LIBRARY_SOURCES} ${PLATFORM_SOURCES})
add_portmino_settings(portmino-core)
add_sanitizers(portmino-core)
set_property(TARGET portmino-core PROPERTY CXX_STANDARD 98)
set_property(TARGET portmino-core PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET portmino-core APPEND PROPERTY COMPILE_OPTIONS "-fpermissive")
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
