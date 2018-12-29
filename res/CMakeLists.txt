# Build the resource pack
set(RESOURCE_SOURCES
    back.png
    board.png
    sfx/piece0.wav
    blocks/blue.png
    blocks/cyan.png
    blocks/green.png
    blocks/gray.png
    blocks/orange.png
    blocks/purple.png
    blocks/red.png
    blocks/white.png)
set(RESOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/base.pk3")

add_custom_command(
    OUTPUT ${RESOURCE_FILE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E tar cf ${RESOURCE_FILE} --format=zip -- ${RESOURCE_SOURCES}
    DEPENDS ${RESOURCE_SOURCES} VERBATIM)
add_custom_target(resources ALL DEPENDS ${RESOURCE_FILE})
