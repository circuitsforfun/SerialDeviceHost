/*
 *  Enumeration Example:
 *      Will enumerate all devices detected on the system and
 *      print out info about the device to the console.
 */

#include "SerialDeviceHost.hpp"
#include "SerialDeviceDiscovery.hpp"

using namespace rw;

int main(int argc, char *argv[]) {
    // Find All Devices
    serial_device::SerialDeviceDiscovery devDisc;
    std::vector<serial_device::deviceList> devices = devDisc.getDeviceList();

    // Print out information to the console about all the found devices
    if (!devices.empty())
    {
        int cnt = 1;
        std::cout << "----------------------------------------------------------------------\n" << std::endl;
        for (const auto& dev : devices)
        {
            std::cout << "Found Device Number: " << cnt << std::endl;
            std::cout << "\tPort: " << dev.commPort << "\tBaud: " << dev.baudRate << std::endl;
            std::cout << "\tDesc: " << dev.name << " - " << dev.info << std::endl;
            std::cout << "\tClass ID: " << dev.class_id << std::endl;
            std::cout << "\tType ID: " << dev.type_id << std::endl;
            std::cout << "\tSerial #: " << dev.serial << std::endl;
            std::cout << "\tVer: " << (int) dev.version_major << "." << (int) dev.version_minor << "." << (int) dev.version_revision << std::endl;
            std::cout << "----------------------------------------------------------------------\n" << std::endl;
            cnt++;
        }
    }
    else
        std::cout << "No Devices Found!" << std::endl;

    return EXIT_SUCCESS;

}
