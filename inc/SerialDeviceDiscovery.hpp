#pragma once

#include "SerialDevice.hpp"
#include "MixedDataType.hpp"

namespace rw
{
    namespace serial_device
    {
        typedef struct deviceList : deviceDescriptor
        {
            std::string commPort;
            uint32_t baudRate;
        } deviceList;

        class SerialDeviceDiscovery : public mdt::MixedDataType
        {
          public:
            SerialDeviceDiscovery();
            ~SerialDeviceDiscovery() override = default;
            std::vector<deviceList> getDeviceList();
            void update();
            void sendPacket(kSudCommandType cmd);

          protected:
            bool getDeviceInfo();
          private:
            deviceDescriptor device_description_;
            serial::Serial *serial_device_;
            std::vector<uint8_t> in_packet_;
            std::vector<uint8_t> out_packet_;
            uint8_t header_byte_;
            uint8_t packet_id_;
            uint8_t stop_byte_;
            uint8_t cmd_{};
            bool has_device_info_{};
            bool data_available_;
            bool data_error_;
            bool device_open_;
            bool found_device_;
            CRC16 crc_;
        };
    }
}

