cmake_minimum_required(VERSION 3.1)
project(SerialDevice_Examples)

set(CMAKE_CXX_STANDARD 14)

find_package(SerialDeviceHost REQUIRED PATHS "${SerialDeviceHost_DIR}")

add_executable(SerialDevice_Enumeration Device_Enumeration.cpp ${SerialDeviceHost_Src})
target_link_libraries(SerialDevice_Enumeration ${SerialDeviceHost_Lib})

add_executable(SerialDevice_LED_Blink LED_Blink.cpp ${SerialDeviceHost_Src})
target_link_libraries(SerialDevice_LED_Blink ${SerialDeviceHost_Lib})
