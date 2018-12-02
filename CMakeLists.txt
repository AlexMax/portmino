cmake_minimum_required(VERSION 3.11.2)

project("portmino" C)

foreach(DIR dep src)
    add_subdirectory(${DIR})
endforeach()
