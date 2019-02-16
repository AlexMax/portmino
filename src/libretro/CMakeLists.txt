set(LIBRETRO_SOURCES
    libretro.c  libretro.h)

add_library(portmino_libretro SHARED ${LIBRETRO_SOURCES})
set_target_properties(portmino_libretro PROPERTIES PREFIX "")
target_link_libraries(portmino_libretro portmino-core basemino)

if(MSVC)
    target_compile_definitions(portmino_libretro PRIVATE _USE_MATH_DEFINES)
endif()
