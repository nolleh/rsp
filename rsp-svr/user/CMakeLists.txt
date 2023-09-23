cmake_minimum_required(VERSION 3.5)
project(User VERSION 1.0)

cmake_policy(SET CMP0048 NEW)

find_package(Boost)
find_package(Protobuf)

configure_file(
  config.h.in
  config.h
)

# source_group("cpp"
#   FILES
#   ${PROJECT_SOURCE_DIR}/src
#   REGULAR_EXPRESSION "*.(cpp | c)"
# )

file(GLOB_RECURSE SRC src/*.cpp)

add_executable(User
  ${SRC}
  ${PROTO}
)

target_include_directories(User PRIVATE
  ${Boost_DIR}/include
  ${Libs_DIR}/include
  ${Proto_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}
)

target_link_directories(User PRIVATE
  ${Boost_DIR}/lib
)

target_link_libraries(User
  Libs
  protobuf
)
