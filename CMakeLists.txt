cmake_minimum_required(VERSION 3.7.2)

project(Portmino C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Global cache values - these are settings that are actually germane to the build process.
set(PORTMINO_EMBED_RESOURCE OFF
    CACHE BOOL "Embed resource file in all targets, including those that don't need it, like SDL.")

# Use folders in Visual Studio.  This helps us hide irrelevant targets in subprojects.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Dependencies
find_package(Sanitizers)
if(NOT EMSCRIPTEN)
    find_package(SDL2)
endif()

# Portmino directories
foreach(DIR dep res src)
    add_subdirectory(${DIR})
endforeach()

# Unit testing
option(BUILD_TESTING "Build unit tests - requires cmocka." OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
