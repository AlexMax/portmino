# Build the resource pack
set(RESOURCE_SOURCES
    background/default/1.png
    block/default/8px.png
    gametype/stdmino/endurance/gametype.lua
    gametype/stdmino/endurance/gametype.cfg
    gametype/stdmino/scoreattack/gametype.lua
    gametype/stdmino/scoreattack/gametype.cfg
    gametype/stdmino/timeattack/gametype.lua
    gametype/stdmino/timeattack/gametype.cfg
    gametype/stdmino/versus/gametype.lua
    gametype/stdmino/versus/gametype.cfg
    interface/default/board.png
    interface/default/font.png
    ruleset/stdmino/gravity.lua
    ruleset/stdmino/next_buffer.lua
    ruleset/stdmino/pieces.cfg
    ruleset/stdmino/randomizer.lua
    ruleset/stdmino/ruleset.cfg
    ruleset/stdmino/ruleset.lua
    script/next.lua
    sfx/default/cursor.wav
    sfx/default/gameover.wav
    sfx/default/go.wav
    sfx/default/lock.wav
    sfx/default/move.wav
    sfx/default/ok.wav
    sfx/default/piece0.wav
    sfx/default/ready.wav
    sfx/default/rotate.wav
    sfx/default/step.wav)
set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/basemino.pk3")

add_custom_command(
    OUTPUT ${RESOURCE_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${RESOURCE_FILE} --format=zip -- ${RESOURCE_SOURCES}
    DEPENDS ${RESOURCE_SOURCES} VERBATIM)
add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE} SOURCES ${RESOURCE_SOURCES})
