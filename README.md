# Serial Device Host

The Host side library for the [Serial Device Peripheral](https://github.com/circuitsforfun/SerialDevicePeripheral) library.

**Some key features:**
* Easily enumerate all devices on the host system
* Supports detailed device information: class, type, serial number, version, name, desc.
* Automatically find devices based on user defined Class, Type and/or Serial Number
* Automatically detect baud rate
* Supports many different data types
* Uses Key / Value pairs with dynamically changeable data types
* Robust to changes in the order of the data or data type to easily maintain compatibility between versions of peripheral and host application
* Easily discover available keys and data types in packets
* Optimized binary packet data for streamlined data throughput 

How to Build
-

1. Download or clone this repo
2. mkdir build
3. cd build
4. cmake ..
5. make

Try out the examples in the build/examples folder

Use in Your Own Project
-

Using CMake add the following lines to your CMakeLists.txt file:

find_path(SerialDeviceHost_DIR "")

find_package(SerialDeviceHost REQUIRED PATHS "${SerialDeviceHost_DIR}")

Then you have a few options, you can set the SerialDeviceHost_DIR environment variable so it's global on your system
or when running cmake you can use the -DSerialDeviceHost_DIR= to set the path to SerialDeviceHost folder.

You'll also need to add SerialDeviceHost_Src to you executable like this:

add_executable(${PROJECT_NAME} main.cpp ${SerialDeviceHost_Src})

and also add the lib to the target_link_libraries like this:

target_link_libraries(${PROJECT_NAME} ${SerialDeviceHost_Lib})


