# Create a static library that contains our resource pack
#
# This complexity is necessary to ensure that the library is properly updated
# anytime we modify any of the resources.

add_executable(xxd xxd.c)
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/basemino.c"
                  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../res"
                  COMMAND "$<TARGET_FILE:xxd>" "-i" "basemino.pk3"
                  ">" "${CMAKE_CURRENT_BINARY_DIR}/basemino.c"
                  DEPENDS "$<TARGET_PROPERTY:resources,BINARY_DIR>/basemino.pk3"
                  COMMENT "Generating basemino.c")
add_library(basemino STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/basemino.c" "${CMAKE_CURRENT_SOURCE_DIR}/basemino.h")
add_dependencies(basemino resources)
target_include_directories(basemino PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
