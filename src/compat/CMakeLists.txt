include(CheckSymbolExists)

set(COMPAT_SOURCES compat.h null.c)

set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")
check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
if(NOT HAVE_ASPRINTF)
    list(APPEND COMPAT_SOURCES "asprintf.c")
endif()

add_library(portmino-compat STATIC ${COMPAT_SOURCES})
set_property(TARGET portmino-compat PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(portmino-compat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(HAVE_ASPRINTF)
    target_compile_definitions(portmino-compat PUBLIC HAVE_ASPRINTF _GNU_SOURCE)
endif()
