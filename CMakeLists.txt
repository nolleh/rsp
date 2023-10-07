cmake_minimum_required(VERSION 3.5)

project(Rsp VERSION 1.0)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(Libs_DIR ${CMAKE_SOURCE_DIR}/rsp-libs)
set(Proto_DIR ${CMAKE_SOURCE_DIR}/gen-proto)

file(GLOB_RECURSE PROTO ${CMAKE_SOURCE_DIR}/gen_proto/*.cc)

add_subdirectory(gen-proto)
add_subdirectory(rsp-libs)
add_subdirectory(rsp-svr)
add_subdirectory(rsp-cli)
