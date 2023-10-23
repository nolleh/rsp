cmake_minimum_required(VERSION 3.5)

project(Room VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

if(NOT DEFINED Libs_DIR)
  set(Libs_DIR ${CMAKE_SOURCE_DIR}/../rsp-libs)
endif()

add_executable(Room
  ${SRC}
)

target_include_directories(Room PRIVATE
  # ${Boost_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${Libs_DIR}/include
  ${Proto_DIR}
  ${PROJECT_SOURCE_DIR}/include
)

target_link_directories(Room PRIVATE
  # ${Boost_DIR}/lib
  ${Boost_LIBRARY_DIRS}
)

target_link_libraries(Room
  ${PROJECT_BINARY_DIR}/../../rsp-libs/libLibs.a
  ${PROJECT_BINARY_DIR}/../../gen-proto/libProto.a
  protobuf
)
