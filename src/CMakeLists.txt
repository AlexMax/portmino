if(MSVC)
    add_compile_options()
else()
    add_compile_options(-Wall -Wextra)
endif()

set(GAME_SOURCES
    event.c         event.h
    field.c         field.h
    game.c          game.h
    picture.c       picture.h
    render.c        render.h
    softrender.c    softrender.h
    state.c         state.h)
set(LIBRARY_SOURCES
                    stb_image.h)

add_library(portmino-core STATIC ${GAME_SOURCES} ${LIBRARY_SOURCES})
add_sanitizers(portmino-core)
set_property(TARGET portmino-core PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(portmino-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(portmino-core lua)
if(UNIX)
    target_link_libraries(portmino-core m)
endif()

add_subdirectory(libretro)
add_subdirectory(sdl)
