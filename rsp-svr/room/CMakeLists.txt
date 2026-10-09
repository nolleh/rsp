cmake_minimum_required(VERSION 3.10)

project(Room VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost CONFIG QUIET)
if(NOT Boost_FOUND)
  if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
  endif()
  find_package(Boost REQUIRED)
endif()
find_package(Protobuf)
find_package(cppzmq)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD)

if(UNIX AND NOT APPLE)
    set(EXTENSION ".so")
elseif(APPLE)
    set(EXTENSION ".dylib")
elseif(WIN32)
    set(EXTENSION ".dll")
else()
    message(SEND_ERROR "unsupported")
endif()

set(Room_CONTENTS_BIN
    ${CMAKE_BINARY_DIR}/rsp-svr/room_contents/libRoomContents${EXTENSION}
)
configure_file(config.h.in ${PROJECT_SOURCE_DIR}/include/room/config.h)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
    set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

if(NOT DEFINED Libs_DIR)
    set(Libs_DIR ${CMAKE_SOURCE_DIR}/../rsp-libs)
endif()

add_executable(Room ${SRC})

# used generator expression
# (notwork) cmake-lint: disable=C0301
# https://cmake.org/cmake/help/latest/manual
# /cmake-generator-expressions.7.html#variable-queries
target_compile_options(Room PRIVATE $<$<CONFIG:Debug>:-g>)

target_include_directories(
    Room
    PRIVATE
        # ${Boost_DIR}/include
        ${Boost_INCLUDE_DIR}
        ${Libs_DIR}/include
        ${Proto_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${rsp-svr_SOURCE_DIR}/rci/include
)

target_link_libraries(
    Room
    Libs
    Proto
    ${Protobuf_LIBRARIES}
    ${Boost_LIBRARIES}
    cppzmq
)

add_executable(
    RoomTest
    test/room_manager_test.cpp
    src/intranet/intranet.cpp
    src/room/room.cpp
    src/room/room_manager.cpp
    src/room/so_manager.cpp
)

target_include_directories(
    RoomTest
    PRIVATE
        ${Boost_INCLUDE_DIR}
        ${Libs_DIR}/include
        ${Proto_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${rsp-svr_SOURCE_DIR}/rci/include
)

target_link_libraries(
    RoomTest
    PRIVATE
    GTest::gtest_main
    Libs
    Proto
    ${Protobuf_LIBRARIES}
    ${Boost_LIBRARIES}
    cppzmq
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(RoomTest PRIVATE -fsanitize=address
                                            -fno-omit-frame-pointer)
    target_link_libraries(RoomTest PRIVATE -fsanitize=address)
endif()

include(GoogleTest)
gtest_discover_tests(RoomTest)

# Exercise actual singleton destruction in a separate process. The fixture
# checks on_unload ordering even if the platform keeps its DSO mapped.
add_library(RoomShutdownContents SHARED test/shutdown_contents.cpp)
target_include_directories(RoomShutdownContents PRIVATE
    ${rsp-svr_SOURCE_DIR}/rci/include)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(RoomShutdownContents PRIVATE -fno-gnu-unique)
endif()

file(GENERATE
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shutdown-include/room/config.h"
    CONTENT "#pragma once\n#define Room_CONTENTS_BIN \"$<TARGET_FILE:RoomShutdownContents>\"\n")

add_executable(RoomShutdownTest
    test/shutdown_test.cpp
    src/intranet/intranet.cpp
    src/room/room.cpp
    src/room/room_manager.cpp
    src/room/so_manager.cpp)
target_include_directories(RoomShutdownTest PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/shutdown-include
    ${Boost_INCLUDE_DIR}
    ${Libs_DIR}/include
    ${Proto_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${rsp-svr_SOURCE_DIR}/rci/include)
target_link_libraries(RoomShutdownTest PRIVATE
    Libs Proto ${Protobuf_LIBRARIES} ${Boost_LIBRARIES} cppzmq)
add_dependencies(RoomShutdownTest RoomShutdownContents)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(RoomShutdownTest PRIVATE
        -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(RoomShutdownTest PRIVATE -fsanitize=address)
endif()

foreach(mode IN ITEMS so-first manager-first channel-first explicit unloaded)
    add_test(NAME RoomShutdown.${mode} COMMAND RoomShutdownTest ${mode})
    set_tests_properties(RoomShutdown.${mode} PROPERTIES TIMEOUT 20)
endforeach()
