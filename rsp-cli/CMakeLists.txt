cmake_minimum_required(VERSION 3.5)
project(Client)

find_package(Boost)

add_subdirectory(libs)

file(GLOB_RECURSE SRC src/*.cpp)

add_executable(Client
  ${SRC}
)
