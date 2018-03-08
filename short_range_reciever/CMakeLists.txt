cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME short_range_reciever)      #change to correct name
project(${PROJECT_NAME})
include_directories(/Users/timlindquist/Documents/Arduino/libraries)

set(${CMAKE_PROJECT_NAME}_BOARD uno)
set(${CMAKE_PROJECT_NAME}_PORT /dev/cu.usbmodem1411)       #leftUSB
#set(${CMAKE_PROJECT_NAME}_PORT /dev/cu.usbmodem1421)       #rightUSB

set(${CMAKE_PROJECT_NAME}_SKETCH short_range_reciever.ino)
generate_arduino_firmware(${CMAKE_PROJECT_NAME})
