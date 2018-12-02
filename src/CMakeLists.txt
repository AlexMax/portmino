set(GAME_SOURCES
    game.c)

add_library(portmino-core STATIC ${GAME_SOURCES})
target_link_libraries(portmino-core lua)

add_subdirectory(libretro)
#add_subdirectory(sdl)
