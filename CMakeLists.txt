cmake_minimum_required(VERSION 3.7.2)

project(Portmino C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Sanitizers)
find_package(SDL2)

foreach(DIR dep src)
    add_subdirectory(${DIR})
endforeach()
