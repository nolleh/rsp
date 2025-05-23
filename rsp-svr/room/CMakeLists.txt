cmake_minimum_required(VERSION 3.5)

project(Room VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)
find_package(cppzmq)

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
    ${CMAKE_BINARY_DIR}/rsp-libs/libLibs.a
    ${CMAKE_BINARY_DIR}/gen-proto/libProto.a
    ${Protobuf_LIBRARIES}
    ${Boost_LIBRARIES}
    cppzmq
)
