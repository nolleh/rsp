cmake_minimum_required(VERSION 3.5)
project(Proto VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Protobuf REQUIRED)

file(GLOB_RECURSE PROTO_FILES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/proto/*.proto
)

set(PROTO_SRCS)
set(PROTO_HDRS)
foreach(PROTO_FILE ${PROTO_FILES})
  file(RELATIVE_PATH PROTO_RELATIVE ${CMAKE_SOURCE_DIR} ${PROTO_FILE})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" PROTO_SOURCE ${PROTO_RELATIVE})
  string(REGEX REPLACE "\\.proto$" ".pb.h" PROTO_HEADER ${PROTO_RELATIVE})

  list(APPEND PROTO_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SOURCE})
  list(APPEND PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER})

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_SOURCE}
      ${CMAKE_CURRENT_BINARY_DIR}/${PROTO_HEADER}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${CMAKE_SOURCE_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating C++ protobuf sources for ${PROTO_RELATIVE}"
    VERBATIM
  )
endforeach()

add_library(Proto ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(Proto PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Protobuf_INCLUDE_DIR}
)

target_link_libraries(Proto
  ${Protobuf_LIBRARIES}
)
