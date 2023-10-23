cmake_minimum_required(VERSION 3.5)
project(User VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)
find_package(cppzmq)

configure_file(
  config.h.in
  ${PROJECT_SOURCE_DIR}/include/user/config.h
)

# source_group("cpp"
#   FILES
#   ${PROJECT_SOURCE_DIR}/src
#   REGULAR_EXPRESSION "*.(cpp | c)"
# )

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen_proto)
endif()

if(NOT DEFINED Libs_DIR)
  set(Libs_DIR ${CMAKE_SOURCE_DIR}/../rsp-libs)
endif()

add_executable(User
  ${SRC}
)

target_include_directories(User PRIVATE
  # ${Boost_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${Libs_DIR}/include
  ${Proto_DIR}
  ${PROJECT_SOURCE_DIR}/include
)

target_link_directories(User PRIVATE
  # ${Boost_DIR}/lib
  ${Boost_LIBRARY_DIRS}
  cppzmq
)

target_link_libraries(User
  ${PROJECT_BINARY_DIR}/../../rsp-libs/libLibs.a
  ${PROJECT_BINARY_DIR}/../../gen-proto/libProto.a
  protobuf
  cppzmq
)
