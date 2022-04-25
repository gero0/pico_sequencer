cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
include(pico_sdk_import.cmake)

project(pico_midi)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB SRC_FILES CONFIGURE_DEPENDS "src/*.cpp") # Load all files in src folder

# rest of your project
add_executable(pico_midi ${SRC_FILES})
target_link_libraries(pico_midi pico_stdlib pico_time pico_sync pico_util pico_bootsel_via_double_reset tinyusb_device tinyusb_board hardware_pwm hardware_timer hardware_i2c)
target_include_directories(pico_midi PRIVATE "./include")
target_include_directories(pico_midi PRIVATE "${PICO_SDK_PATH}/lib/tinyusb/src/class")

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pico_midi)
pico_enable_stdio_usb(pico_midi 1)