project(Room VERSION 1.0)

cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SRC src/*.(cpp | c))

add_executable(Room
  src/main.cpp
)
