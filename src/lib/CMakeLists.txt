set(LIBRARY_SOURCES
                    dr_wav.h
    miniz.c         miniz.h
                    stb_image.h)
add_library(portmino-lib STATIC ${LIBRARY_SOURCES})
target_include_directories(portmino-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
