if(MSVC)
    add_compile_options()
else()
    add_compile_options(-Wall -Wextra)
endif()

set(GAME_SOURCES
    event.c event.h
    field.c field.h
    game.c  game.h
    state.c state.h)

add_library(portmino-core STATIC ${GAME_SOURCES})
target_include_directories(portmino-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(portmino-core lua)

add_subdirectory(libretro)
add_subdirectory(sdl)
