/*
 *  LED Blink Example:
 *      Works with the Test Device Peripheral example.
 *      Expects a Class ID 0x0A0A and a Type ID 0x0D0D
 */

#include "SerialDeviceHost.hpp"

using namespace rw;

int main(int argc, char *argv[])
{
    // Look for device with ClassID: 0x0A0A and TypeID: 0x0D0D, it will scan all available port to find a match
    serial_device::SerialDeviceHost myDevice(  0x0A0A, 0x0D0D);

    // You could also specify the port like this on Windows:
    //serial_device::SerialDeviceHost myDevice( "COM3", 0x0A0A, 0x0D0D);

    // Check to make sure the device was found and the port was able to be opened
    if (!myDevice.isOpen() || !myDevice.isFound())
    {
        std::cout << "Error Opening Port or Device Not Found" << std::endl;
        return EXIT_FAILURE;
    }

    // Print Device Info
    std::cout << "Found Device On Port: " << myDevice.getPortName() << std::endl;
    std::cout << "  Description: " << myDevice.getName() << " - " << myDevice.getInfo() << std::endl;
    std::cout << "  Class ID: " << myDevice.getClassID() << std::endl;
    std::cout << "  Type ID: " << myDevice.getTypeID() << std::endl;
    std::cout << "  Serial #: " << myDevice.getSerial() << std::endl;
    std::cout << "  Ver: " << (int)myDevice.getVersionMajor() << "." << (int)myDevice.getVersionMinor() << "." << (int)myDevice.getVersionRev() << std::endl;

    // Setup some variables to hold our command and state value
    uint8_t ledState = 0;
    myDevice.clearData();
    myDevice.add("cmd", "led");
    myDevice.add("sta", ledState);
    myDevice.sendPacket();

    for(int i = 1; i < 100; i++)
    {
        ledState ^= 1;
        myDevice.add("sta", ledState); // Since this already exists it will just modify the value instead of adding it
        myDevice.sendPacket();
        myDevice.delay_ms(250);
        // Must call this update periodically to process SerialDeviceHost routines
        myDevice.update();
    }

    return EXIT_SUCCESS;
}
