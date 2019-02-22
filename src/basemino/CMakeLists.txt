# Create a static library that contains our resource pack
#
# This complexity is necessary to ensure that the library is properly updated
# anytime we modify any of the resources.

find_program(XXD xxd)
if(XXD)
    set(XXD_BINARY "${XXD}")
else()
    if(CMAKE_CROSSCOMPILING)
        message(FATAL_ERROR "Can't find xxd, and can't compile xxd because we're cross-compiling.")
    endif()
    add_executable(xxd xxd.c)
    set(XXD_BINARY "$<TARGET_FILE:xxd>")
endif()

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/basemino.c"
                  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../res"
                  COMMAND "${XXD_BINARY}" "-i" "basemino.pk3"
                  ">" "${CMAKE_CURRENT_BINARY_DIR}/basemino.c"
                  DEPENDS "$<TARGET_PROPERTY:resources,BINARY_DIR>/basemino.pk3"
                  COMMENT "Generating basemino.c")
add_library(basemino STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/basemino.c" "${CMAKE_CURRENT_SOURCE_DIR}/basemino.h")
add_dependencies(basemino resources)
target_include_directories(basemino PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
