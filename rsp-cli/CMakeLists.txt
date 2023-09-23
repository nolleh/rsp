cmake_minimum_required(VERSION 3.5)
project(Client)

find_package(Boost)
find_package(Protobuf)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC src/*.cpp)
if (NOT DEFINED PROTO) 
  file(GLOB_RECURSE PROTO ${CMAKE_SOURCE_DIR}/../gen_proto/proto/*.cc)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

add_executable(Client
  ${SRC}
  ${PROTO}
)

target_include_directories(Client PRIVATE
  ${Proto_DIR}
  ${Libs_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Client PRIVATE
  protobuf
)
