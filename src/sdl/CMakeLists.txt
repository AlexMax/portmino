set(SDL_SOURCES
    main.c)

add_executable(portmino ${SDL_SOURCES})
target_link_libraries(portmino portmino-core)
