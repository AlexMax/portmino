# Build the resource pack
set(RESOURCE_SOURCES
    background/default/1.png
    block/default/8px.png
    interface/default/board.png
    interface/default/font.png
    sfx/default/lock.wav
    sfx/default/move.wav
    sfx/default/piece0.wav
    sfx/default/rotate.wav
    sfx/default/step.wav)
set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/basemino.pk3")

add_custom_command(
    OUTPUT ${RESOURCE_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${RESOURCE_FILE} --format=zip -- ${RESOURCE_SOURCES}
    DEPENDS ${RESOURCE_SOURCES} VERBATIM)
add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE})
