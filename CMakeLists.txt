cmake_minimum_required(VERSION 3.13)

include(ExternalProject)

ExternalProject_Add(
    external SOURCE_DIR "${CMAKE_SOURCE_DIR}/test"
    PREFIX ${CMAKE_BINARY_DIR}/test
    BINARY_DIR ${CMAKE_BINARY_DIR}/test
    INSTALL_COMMAND "" 
    BUILD_ALWAYS 1
)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include(pico_sdk_import.cmake)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

project(pico_midi LANGUAGES CXX C)


add_subdirectory(./firmware)
