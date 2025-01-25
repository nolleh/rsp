cmake_minimum_required(VERSION 3.5)

project(RoomContents VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
    set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

add_library(RoomContents SHARED ${SRC})


# used generator expression
# (notwork) cmake-lint: disable=C0301
# https://cmake.org/cmake/help/latest/manual
# /cmake-generator-expressions.7.html#variable-queries
target_compile_options(RoomContents PRIVATE $<$<CONFIG:Debug>:-g>)

target_include_directories(
  RoomContents
  PRIVATE
  ${Boost_INCLUDE_DIR}
  ${Proto_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${rsp-svr_SOURCE_DIR}/rci/include
)

target_link_libraries(
  RoomContents
  ${Protobuf_LIBRARIES}
  ${Boost_LIBRARIES}
)
