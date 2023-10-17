cmake_minimum_required(VERSION 3.5)
project(Libs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_policy(SET CMP0135 NEW)

find_package(Boost)
find_package(cppzmq)
# find_package(Protobuf)

# donwload gtest
# https://google.github.io

set(BASE_URL https://github.com/google/googletest/archive)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL ${BASE_URL}/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen-proto)
endif()


## add cpp
add_library(Libs
  ${SRC}
)

target_include_directories(Libs PUBLIC
  # ${Boost_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Proto_DIR}
  cppzmq
)

target_link_directories(Libs PUBLIC
  # ${Boost_DIR}/lib
  ${Boost_LIBRARY_DIRS}
  ${PROJECT_BINARY_DIR}/../gen-proto/libProto.a
  cppzmq
)

enable_testing()

file(GLOB_RECURSE TEST test/*.cpp)

add_executable(LibsTest
  ${TEST}
)

target_include_directories(LibsTest PRIVATE
  ${PROJECT_BINARY_DIR}
)

target_link_libraries(LibsTest PRIVATE
  GTest::gtest_main
  Libs
  ## added because link fail
  protobuf
  ${PROJECT_BINARY_DIR}/../gen-proto/libProto.a
)

include(GoogleTest)
gtest_discover_tests(LibsTest)
