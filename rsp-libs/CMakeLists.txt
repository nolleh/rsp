cmake_minimum_required(VERSION 3.10)
project(Libs)

cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost CONFIG REQUIRED)
find_package(cppzmq)
find_package(Protobuf)

# download gtest
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

# GoogleTest 1.14 supports C++17 and avoids its char8_t conversion warning
# when it is not compiled as part of the project's C++20 targets.
foreach(_RSP_GTEST_TARGET gtest gtest_main gmock gmock_main)
  if(TARGET ${_RSP_GTEST_TARGET})
    set_target_properties(${_RSP_GTEST_TARGET} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
    )
  endif()
endforeach()

file(GLOB_RECURSE SRC src/*.cpp)
if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen-proto)
endif()


## add cpp
add_library(Libs
  ${SRC}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
  # cppzmq 4.10.0 uses the pre-C++11 spelling for user-defined literals.
  # Limit this workaround to Libs, where the external header is compiled.
  target_compile_options(Libs PRIVATE -Wno-deprecated-literal-operator)
endif()

target_link_libraries(Libs PUBLIC Proto)

target_include_directories(Libs PUBLIC
  # ${Boost_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${Proto_DIR}
  ${cppzmq_INCLUDE_DIR}
  ${Protobuf_INCLUDE_DIR}
)

target_link_directories(Libs PUBLIC
  # ${Boost_DIR}/lib
  ${Boost_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/gen-proto
  # protobuf
)

enable_testing()

file(GLOB_RECURSE TEST test/*.cpp)

add_executable(LibsTest
  ${TEST}
)

target_include_directories(LibsTest PRIVATE
  ${CMAKE_BINARY_DIR}
)

target_link_directories(LibsTest PRIVATE
  ${Boost_LIBRARY_DIRS}
)

target_link_libraries(LibsTest PRIVATE
  GTest::gtest_main
  Libs
  ${Protobuf_LIBRARIES}
  cppzmq
)

include(GoogleTest)
gtest_discover_tests(LibsTest)
