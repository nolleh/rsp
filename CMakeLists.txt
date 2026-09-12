cmake_minimum_required(VERSION 3.5)

project(Rsp VERSION 1.0)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    find_program(XCRUN_EXECUTABLE xcrun)
    if(NOT XCRUN_EXECUTABLE)
        message(FATAL_ERROR "xcrun is required to locate the macOS SDK")
    endif()

    execute_process(
        COMMAND ${XCRUN_EXECUTABLE} --sdk macosx --show-sdk-path
        OUTPUT_VARIABLE MACOS_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE XCRUN_RESULT
    )
    if(NOT XCRUN_RESULT EQUAL 0 OR NOT IS_DIRECTORY "${MACOS_SDK_PATH}")
        message(FATAL_ERROR "Failed to locate the macOS SDK with xcrun")
    endif()

    add_compile_options(
        -isysroot "${MACOS_SDK_PATH}"
        -isystem "${MACOS_SDK_PATH}/usr/include/c++/v1"
    )
endif()

set(Libs_DIR ${CMAKE_SOURCE_DIR}/rsp-libs)
set(Proto_DIR ${CMAKE_BINARY_DIR}/gen-proto)

add_subdirectory(gen-proto)
add_subdirectory(rsp-libs)
add_subdirectory(rsp-svr)
add_subdirectory(rsp-cli)
