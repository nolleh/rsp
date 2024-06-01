cmake_minimum_required(VERSION 3.5)
project(Proto VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf)

file(GLOB_RECURSE PROTO ${PROJECT_SOURCE_DIR}/proto/*.cc)

add_library(Proto
  ${PROTO}
)

target_include_directories(Proto PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${Protobuf_INCLUDE_DIR}
)

target_link_libraries(Proto
  ${Protobuf_LIBRARIES}
)
