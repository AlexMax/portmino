set(LIBRARY_SOURCES
    dr_wav.c        dr_wav.h
                    khash.h
    miniz.c         miniz.h
    mpack.c         mpack.h
    stb_image.c     stb_image.h)
add_library(portmino-lib STATIC ${LIBRARY_SOURCES})
set_property(TARGET portmino-lib PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(portmino-lib PUBLIC MINIZ_NO_STDIO)
target_include_directories(portmino-lib SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
