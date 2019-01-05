# Build the resource pack
set(RESOURCE_SOURCES
    background/default/1.png
    block/default/blue.png
    block/default/cyan.png
    block/default/gray.png
    block/default/green.png
    block/default/orange.png
    block/default/purple.png
    block/default/red.png
    block/default/white.png
    block/default/yellow.png
    interface/default/board.png
    sfx/default/piece0.wav)
set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/basemino.pk3")

add_custom_command(
    OUTPUT ${RESOURCE_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${RESOURCE_FILE} --format=zip -- ${RESOURCE_SOURCES}
    DEPENDS ${RESOURCE_SOURCES} VERBATIM)
add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE})
