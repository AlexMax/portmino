set(SDL_SOURCES
    main.c)

add_executable(portmino ${SDL_SOURCES})
add_sanitizers(portmino)
if(EMSCRIPTEN)
    if(PORTMINO_EMBED_RESOURCE)
        message(WARNING "Emscripten already embeds resource file using its own mechanism.")
    endif()

    # Emscripten comes with SDL2 built-in
    target_compile_options(portmino PUBLIC -s USE_SDL=2)
    target_link_libraries(portmino portmino-core)
    set_target_properties(portmino PROPERTIES LINK_FLAGS "-s USE_SDL=2  --preload-file ../../../res/@/usr/share/portmino/basemino/")
    set_target_properties(portmino PROPERTIES SUFFIX ".html")
else()
    target_link_libraries(portmino portmino-core SDL2::SDL2main SDL2::SDL2)
    if(PORTMINO_EMBED_RESOURCE)
        target_compile_definitions(portmino PRIVATE MINO_EMBED_RESOURCE)
        target_link_libraries(portmino basemino)
    endif()
endif()
