project(rsp-svr)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)

add_subdirectory(libs)
add_subdirectory(user)
add_subdirectory(room)

