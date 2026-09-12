cmake_minimum_required(VERSION 3.5)
project(Client)

find_package(Boost)
find_package(Protobuf)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS -rdynamic)

if(NOT DEFINED PROTO)
  set(Proto_DIR ${CMAKE_SOURCE_DIR}/../gen-proto)
endif()

if(NOT DEFINED Libs_DIR)
  set(Libs_DIR ${CMAKE_SOURCE_DIR}/../rsp-libs)
endif()

add_executable(Client
  src/main.cpp
  src/client_session.cpp
  src/state/factory.cpp
  src/state/state.cpp
)

target_include_directories(Client PRIVATE
  ${Boost_INCLUDE_DIR}
  ${Proto_DIR}
  ${Libs_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Client PRIVATE
  ${Boost_LIBRARY_DIRS}
  Libs
  Proto
  ${Protobuf_LIBRARIES}
)

add_executable(ClientTest
  test/client_session_test.cpp
  test/state_in_room_test.cpp
  src/client_session.cpp
  src/state/factory.cpp
  src/state/state.cpp
)

target_include_directories(ClientTest PRIVATE
  ${Boost_INCLUDE_DIR}
  ${Proto_DIR}
  ${Libs_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ClientTest PRIVATE
  GTest::gtest_main
  Libs
  Proto
  ${Protobuf_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(ClientTest)
