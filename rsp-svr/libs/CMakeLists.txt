cmake_minimum_required(VERSION 3.5)
project(Libs)

find_package(Boost)

file(GLOB_RECURSE SRC src/*.cpp)

## add cpp
add_library(Libs
  ${SRC}
)

target_include_directories(Libs PRIVATE
  ${Boost_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_directories(Libs PRIVATE
  ${Boost_DIR}/lib
)
