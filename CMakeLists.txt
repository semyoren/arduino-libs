cmake_minimum_required(VERSION 3.29)
project(arduino_libs)

set(CMAKE_CXX_STANDARD 14)

include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-20240930.arduino3/tools/sdk/esp32s3/include/freertos/include/freertos)
include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-20240930.arduino3/cores/esp32)
include_directories(/Users/semyonkotliarenko/Library/Arduino15/packages/arduino/hardware/esp32/2.0.18-20240930.arduino3/libraries/WiFi/src)
include_directories(ota_resolver/src)

add_executable(arduino_libs
    ota_resolver/src/OTA_Resolver.cpp
    ota_resolver/src/OTA_Resolver.h)
