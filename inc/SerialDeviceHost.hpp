#pragma once

#include "MixedDataType.hpp"
#include "SerialDevice.hpp"

namespace rw
{
    namespace serial_device
    {
        class SerialDeviceHost : public mdt::MixedDataType
        {
          public:
            SerialDeviceHost(uint16_t classID, uint16_t typeID, uint32_t serialNum = kSerialNumberAny, uint32_t baudRate = 115200);
            explicit SerialDeviceHost(const std::string &commPort, uint16_t classID, uint16_t typeID, uint32_t serialNum = kSerialNumberAny, uint32_t baudRate = 115200);
            ~SerialDeviceHost() override;
            uint16_t getClassID() const;
            uint16_t getTypeID() const;
            uint32_t getSerial() const;
            uint8_t getVersionMajor() const;
            uint8_t getVersionMinor() const;
            uint8_t getVersionRev() const;
            std::string getName() const;
            std::string getInfo() const;
            std::string getPortName() const;
            void update();
            void sendPacket(kSudCommandType cmd = SD_COMMAND_SEND_DATA);
            bool available() const;
            bool isError() const;
            bool isFound() const;
            bool isOpen() const;
            void delay_ms(unsigned long milliseconds);

          protected:
            bool findDevice(uint16_t classID, uint16_t typeID, uint32_t serialNum, uint32_t baudRate);
            bool portExists(const std::string &commPort);
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
    } // end namespace serial_device
} // end namespace rw
