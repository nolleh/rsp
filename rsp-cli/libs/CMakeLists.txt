
cmake_minimum_required(VERSION 3.5)
project(Libs)

find_package(Boost)
file(GLOB_RECURSE SRC src/*.cpp)
# add_library(Libs
#   ${SRC}
# )

# target_include_directories(Libs PRIVATE
#   ${CMAKE_CURRENT_SOURCE_DIR}/include
# )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
