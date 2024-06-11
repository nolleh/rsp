cmake_minimum_required(VERSION 3.5)
project(User VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)
find_package(cppzmq)

configure_file(config.h.in ${PROJECT_SOURCE_DIR}/include/user/config.h)

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

add_executable(User ${SRC})

# used generator expression
# lint disable (not work...hum...)
# cmake-lint: disable=C0301
# https://cmake.org/cmake/help/latest/manual
# /cmake-generator-expressions.7.html#variable-queries
target_compile_options(User PRIVATE $<$<CONFIG:Debug>:-g>)

target_include_directories(
    User
    PRIVATE
        # ${Boost_DIR}/include
        ${Boost_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        ${Libs_DIR}/include
        ${Proto_DIR}
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
    User
    ${CMAKE_BINARY_DIR}/rsp-libs/libLibs.a
    ${CMAKE_BINARY_DIR}/gen-proto/libProto.a
    ${Boost_LIBRARIES}
    ${Protobuf_LIBRARIES}
    cppzmq
)
