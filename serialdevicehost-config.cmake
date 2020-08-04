
# ======================================================
#  Version variables:
# ======================================================
SET(SERIAL_DEVICE_HOST_VERSION 1.0.0)
SET(SERIAL_DEVICE_HOST_VERSION_MAJOR  1)
SET(SERIAL_DEVICE_HOST_VERSION_MINOR  0)
SET(SERIAL_DEVICE_HOST_VERSION_PATCH  0)
SET(SERIAL_DEVICE_HOST_VERSION_TWEAK  0)
SET(SERIAL_DEVICE_HOST_VERSION_STATUS "")

set(serial_DIR "${CMAKE_CURRENT_LIST_DIR}/third-party/serial")
include_directories("${serial_DIR}/include")
include_directories("${CMAKE_CURRENT_LIST_DIR}/inc")

set(SerialDeviceHost_Src
        ${serial_DIR}/src/serial.cc
        )
if(APPLE)
    # If OSX
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/unix.cc)
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/list_ports/list_ports_osx.cc)
    set (SerialDeviceHost_Lib "")
elseif(UNIX)
    # If unix
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/unix.cc)
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/list_ports/list_ports_linux.cc)
    set (SerialDeviceHost_Lib "")
else()
    # If windows
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/win.cc)
    list(APPEND SerialDeviceHost_Src ${serial_DIR}/src/impl/list_ports/list_ports_win.cc)
    set (SerialDeviceHost_Lib setupapi.lib)
endif()

list(APPEND SerialDeviceHost_Src "${CMAKE_CURRENT_LIST_DIR}/src/SerialDeviceHost.cpp")
list(APPEND SerialDeviceHost_Src "${CMAKE_CURRENT_LIST_DIR}/src/SerialDeviceDiscovery.cpp")


