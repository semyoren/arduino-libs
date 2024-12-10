cmake_minimum_required(VERSION 3.29)
project(arduino_libs)

set(CMAKE_CXX_STANDARD 14)

include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-arduino.5/tools/sdk/esp32s3/include/freertos/include/freertos)
include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-arduino.5/cores/esp32)
include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-arduino.5/libraries/WiFi/src)
include_directories(OTAResolver/src)

add_executable(arduino_libs
        OTAResolver/src/OTAResolver.cpp
        OTAResolver/src/OTAResolver.h)
