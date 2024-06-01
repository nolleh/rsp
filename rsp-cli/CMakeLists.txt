cmake_minimum_required(VERSION 3.5)
project(Client)

find_package(Boost)
find_package(Protobuf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS -rdynamic)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen-proto)
endif()

if(NOT DEFINED Libs_DIR)
  set(Libs_DIR ${CMAKE_SOURCE_DIR}/../rsp-libs)
endif()

add_executable(Client
  ${SRC}
)

target_include_directories(Client PRIVATE
  ${Boost_INCLUDE_DIR}
  ${Proto_DIR}
  ${Libs_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Client PRIVATE
  ${Boost_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/rsp-libs/libLibs.a
  ${CMAKE_BINARY_DIR}/gen-proto/libProto.a
  ${Protobuf_LIBRARIES}
)
