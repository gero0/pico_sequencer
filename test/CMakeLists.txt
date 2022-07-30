cmake_minimum_required(VERSION 3.13)
project(pico_midi_test)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# CppUTest
include(FetchContent)
FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        latest-passing-build # or use release tag, eg. v3.8
)
# Set this to ON if you want to have the CppUTests in your project as well.
set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")
FetchContent_MakeAvailable(CppUTest)

file(GLOB test_src_files CONFIGURE_DEPENDS
 "${CMAKE_SOURCE_DIR}/*.cpp" 
 "${CMAKE_SOURCE_DIR}/mock/*.cpp" 
 "${CMAKE_SOURCE_DIR}/mock/hardware/*.cpp" 
 "${CMAKE_SOURCE_DIR}/../firmware/src/midi.cpp"
 "${CMAKE_SOURCE_DIR}/../firmware/src/sequence.cpp"
)

add_executable(run_tests "${test_src_files}")

file(GLOB include_files CONFIGURE_DEPENDS 
  "${CMAKE_SOURCE_DIR}/mock"
  "${CMAKE_SOURCE_DIR}/mock/hardware"
  "${CMAKE_SOURCE_DIR}/../firmware/include"
)

target_include_directories(run_tests PRIVATE "${include_files}")

target_link_libraries(run_tests PRIVATE CppUTest CppUTestExt)

add_custom_command(TARGET run_tests COMMAND ./run_tests POST_BUILD)




