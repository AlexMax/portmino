cmake_minimum_required(VERSION 3.7.2)

project(Portmino C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Global cache values
set(PORTMINO_EMBED_RESOURCE OFF
    CACHE BOOL "Embed resource file in all targets, including those that don't need it, like SDL.")

# Use folders in Visual Studio.  This helps us hide irrelevant targets in subprojects.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Force colored output with GCC/Clang.  This ensures that we get colored output with Ninja.
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(${FORCE_COLORED_OUTPUT})
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()
endif()

# Dependencies
find_package(Sanitizers)
find_package(SDL2)

# Unit testing
include(CTest)

foreach(DIR dep res src test)
    add_subdirectory(${DIR})
endforeach()
