cmake_minimum_required(VERSION 3.5)
project(Libs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost)
# find_package(Protobuf)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

## add cpp
add_library(Libs
  ${SRC}
)

target_include_directories(Libs PRIVATE
  ${Boost_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
  ${Proto_DIR}
)

target_link_directories(Libs PRIVATE
  ${Boost_DIR}/lib
  ${PROJECT_BINARY_DIR}/../../gen_proto/libProto.a
  protobuf
)
