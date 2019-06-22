# Some stuff to remember about ExternalProject
#
# - BUILD_BYPRODUCTS is necessary for Ninja to pick up on the dependency.
# - In custom build steps, be explicit about both DEPENDEES and DEPENDERS or
#   else the build step might not be executed after a clean.

include(ExternalProject)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/local/include")

## cmocka
set(CMOCKA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/local/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cmocka-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(cmocka
    URL "https://cmocka.org/files/1.1/cmocka-1.1.3.tar.xz"
    URL_HASH "SHA256=43eabcf72a9c80e3d03f7c8a1c04e408c18d2db5121eb058a3ef732a9dfabfaf"
    CMAKE_ARGS
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/local"
        "-DWITH_STATIC_LIB=ON"
    BUILD_BYPRODUCTS ${CMOCKA_LIBRARY}
    EXCLUDE_FROM_ALL ON)

add_library(cmocka-x STATIC IMPORTED GLOBAL)
add_dependencies(cmocka-x cmocka)
set_target_properties(cmocka-x PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/local/include"
    IMPORTED_LOCATION ${CMOCKA_LIBRARY})

## Lua
set(LUA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/local/lib/${CMAKE_STATIC_LIBRARY_PREFIX}lua${CMAKE_STATIC_LIBRARY_SUFFIX}")
ExternalProject_Add(lua
    URL "http://www.lua.org/ftp/lua-5.3.5.tar.gz"
    URL_HASH "SHA256=0c2eed3f960446e1a3e4b9a1ca2f3ff893b6ce41942cf54d5dd59ab4b3b058ac"
    CMAKE_ARGS
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/local"
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
if(MSVC)
    set(PHYSFS_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/local/lib/${CMAKE_STATIC_LIBRARY_PREFIX}physfs-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
else()
    set(PHYSFS_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/local/lib/${CMAKE_STATIC_LIBRARY_PREFIX}physfs${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()
ExternalProject_Add(physfs
    URL "https://hg.icculus.org/icculus/physfs/archive/de1ea3b2cfc4.tar.gz"
    URL_HASH "SHA256=eb3f6530fe6eb59145094a8cadc88268c633733e6cd32ec033516f320024d655"
    CMAKE_ARGS
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/local"
        "-DPHYSFS_ARCHIVE_7Z=OFF"
        "-DPHYSFS_ARCHIVE_GRP=OFF"
        "-DPHYSFS_ARCHIVE_HOG=OFF"
        "-DPHYSFS_ARCHIVE_ISO9660=OFF"
        "-DPHYSFS_ARCHIVE_MVL=OFF"
        "-DPHYSFS_ARCHIVE_QPAK=OFF"
        "-DPHYSFS_ARCHIVE_SLB=OFF"
        "-DPHYSFS_ARCHIVE_VDF=OFF"
        "-DPHYSFS_ARCHIVE_WAD=OFF"
        "-DPHYSFS_BUILD_SHARED=OFF"
    CMAKE_CACHE_ARGS "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true"
    BUILD_BYPRODUCTS ${PHYSFS_LIBRARY}
    EXCLUDE_FROM_ALL ON)

add_library(physfs-x STATIC IMPORTED GLOBAL)
add_dependencies(physfs-x physfs)
set_target_properties(physfs-x PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/local/include"
    IMPORTED_LOCATION ${PHYSFS_LIBRARY})
