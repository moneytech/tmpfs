cmake_minimum_required(VERSION 3.14)
project(tmpfs C)

set(CMAKE_C_STANDARD 11)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
        "${CMAKE_SOURCE_DIR}/CMakeModules/")
set (CMAKE_C_FLAGS_DEBUG
        "-Wall -ggdb")
include_directories(inc)
find_package(FUSE REQUIRED)
include_directories(PRIVATE ${FUSE_INCLUDE_DIR})
link_libraries(${FUSE_LIBRARIES})
add_compile_definitions(_FILE_OFFSET_BITS=64)


add_executable(tmpfs
        inc/utils.h
        src/tmpfs.c
        src/utils.c)
