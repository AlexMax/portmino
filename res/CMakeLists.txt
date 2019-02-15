# Build the resource pack
set(RESOURCE_SOURCES
    background/default/1.png
    block/default/8px.png
    gametype/default/endurance/main.lua
    gametype/default/endurance/manifest.cfg
    gametype/default/scoreattack/main.lua
    gametype/default/scoreattack/manifest.cfg
    gametype/default/timeattack/main.lua
    gametype/default/timeattack/manifest.cfg
    gametype/default/versus/main.lua
    gametype/default/versus/manifest.cfg
    interface/default/board.png
    interface/default/font.png
    ruleset/default/main.lua
    ruleset/default/pieces.lua
    sfx/default/cursor.wav
    sfx/default/gameover.wav
    sfx/default/lock.wav
    sfx/default/move.wav
    sfx/default/ok.wav
    sfx/default/piece0.wav
    sfx/default/rotate.wav
    sfx/default/step.wav)
set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/basemino.pk3")

add_custom_command(
    OUTPUT ${RESOURCE_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${RESOURCE_FILE} --format=zip -- ${RESOURCE_SOURCES}
    DEPENDS ${RESOURCE_SOURCES} VERBATIM)
add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE} SOURCES ${RESOURCE_SOURCES})
