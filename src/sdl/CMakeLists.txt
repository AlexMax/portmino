set(SDL_SOURCES
    main.c)

add_executable(portmino WIN32 ${SDL_SOURCES})
add_sanitizers(portmino)
if(EMSCRIPTEN)
    # Emscripten comes with SDL2 built-in
    target_compile_options(portmino PUBLIC -s USE_SDL=2)
    target_link_libraries(portmino portmino-core)
    set_target_properties(portmino PROPERTIES LINK_FLAGS "-s USE_SDL=2")
else()
    target_link_libraries(portmino portmino-core SDL2::SDL2main SDL2::SDL2)
endif()
