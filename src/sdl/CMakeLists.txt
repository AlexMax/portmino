set(SDL_SOURCES
    main.c)

add_executable(portmino WIN32 ${SDL_SOURCES})
target_link_libraries(portmino portmino-core SDL2::SDL2main SDL2::SDL2)
