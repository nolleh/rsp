cmake_minimum_required(VERSION 3.5)
project(Proto VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Protobuf)

file(GLOB_RECURSE PROTO ${PROJECT_SOURCE_DIR}/proto/*.cc)

add_library(Proto
  ${PROTO}
)

target_include_directories(Proto PUBLIC
  ${PROJECT_SOURCE_DIR}
)

target_link_libraries(Proto
  protobuf
)
