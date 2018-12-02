set(GAME_SOURCES
    game.c)

add_library(portmino-core STATIC ${GAME_SOURCES})

add_subdirectory(libretro)
#add_subdirectory(sdl)
