# Some stuff to remember about ExternalProject
#
# - BUILD_BYPRODUCTS is necessary for Ninja to pick up on the dependency.
# - In custom build steps, be explicit about both DEPENDEES and DEPENDERS or
#   else the build step might not be executed after a clean.

include(ExternalProject)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/local/include")

## cmocka
ExternalProject_Add(cmocka
    URL "https://cmocka.org/files/1.1/cmocka-1.1.3.tar.xz"
    CMAKE_ARGS "-DWITH_STATIC_LIB=ON" "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/local"
    EXCLUDE_FROM_ALL ON)

## libsamplerate
#add_subdirectory(libsamplerate-master EXCLUDE_FROM_ALL)
#set_target_properties(
#    callback_hang_test
#    callback_test
#    clone_test
#    downsample_test
#    float_short_test
#    misc_test
#    multi_channel_test
#    multichan_throughput_test
#    nullptr_test
#    reset_test
#    simple_test
#    snr_bw_test
#    streaming_test
#    termination_test
#    throughput_test
#    timewarp-file
#    varispeed-play
#    varispeed_test
#    PROPERTIES
#    EXCLUDE_FROM_DEFAULT_BUILD 1
#    FOLDER ExcludedTargets)

## Lua
set(LUA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/local/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lua${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(lua
    URL "http://www.lua.org/ftp/lua-5.3.5.tar.gz"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/local"
    BUILD_BYPRODUCTS ${LUA_LIBRARY}
    EXCLUDE_FROM_ALL ON)
ExternalProject_Add_Step(lua copy-cmake
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/buildlua.cmake ${CMAKE_CURRENT_BINARY_DIR}/lua-prefix/src/lua/CMakeLists.txt
    DEPENDEES download DEPENDERS configure
    COMMENT "Copy CMakeLists.txt")

add_library(lua-x STATIC IMPORTED GLOBAL)
add_dependencies(lua-x lua)
set_target_properties(lua-x PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/local/include"
    IMPORTED_LOCATION ${LUA_LIBRARY})

## PhysicsFS

# Disable everything except .zip support.
set(PHYSFS_ARCHIVE_7Z FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_GRP FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_WAD FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_HOG FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_MVL FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_QPAK FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_SLB FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_ISO9660 FALSE CACHE BOOL "")
set(PHYSFS_ARCHIVE_VDF FALSE CACHE BOOL "")
add_subdirectory(physfs-c17f025e7a92 EXCLUDE_FROM_ALL)
set_property(TARGET physfs-static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(physfs-static PUBLIC "physfs-c17f025e7a92/src")
set_target_properties(
    physfs
    test_physfs
    PROPERTIES
    EXCLUDE_FROM_DEFAULT_BUILD 1
    FOLDER ExcludedTargets)
