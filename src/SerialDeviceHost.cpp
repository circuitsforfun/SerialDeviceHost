#include "SerialDeviceHost.hpp"

namespace rw
{
    namespace serial_device
    {
        SerialDeviceHost::SerialDeviceHost(uint16_t classID, uint16_t typeID, uint32_t serialNum, uint32_t baudRate)
        {
            found_device_ = false;
            device_open_ = false;
            data_available_ = false;
            data_error_ = false;
            header_byte_ = 0xAA;
            packet_id_ = 0xBB;
            stop_byte_ = 0xDD;
            serial_device_ = nullptr;
            found_device_ = findDevice(classID, typeID, serialNum, baudRate);
        }

        SerialDeviceHost::SerialDeviceHost(const std::string &commPort, uint16_t classID, uint16_t typeID, uint32_t serialNum, uint32_t baudRate)
        {
            found_device_ = false;
            device_open_ = false;
            data_available_ = false;
            data_error_ = false;
            header_byte_ = 0xAA;
            packet_id_ = 0xBB;
            stop_byte_ = 0xDD;
            serial_device_ = nullptr;
            if (portExists(commPort))
            {
                serial_device_ = new serial::Serial(commPort, baudRate, serial::Timeout::simpleTimeout(400));
                if (serial_device_->isOpen())
                {
                    bool checkInfo = getDeviceInfo();
                    if (!checkInfo)
                    {
                        found_device_ = false;
                    } else
                    {
                        if (classID == device_description_.class_id && typeID == device_description_.type_id)
                        {
                            if (serialNum != 0)
                            {
                                if (serialNum == device_description_.serial)
                                    found_device_ = true;
                                else
                                    found_device_ = false;
                            } else
                                found_device_ = true;
                        }
                    }
                } else
                {
                    found_device_ = false;
                }
            }
        }

        bool SerialDeviceHost::portExists(const std::string &commPort)
        {
            std::vector<serial::PortInfo> devices_found = serial::list_ports();
            for (const auto& d : devices_found)
            {
                if (d.port == commPort)
                    return true;
            }
            return false;
        }

        bool SerialDeviceHost::findDevice(uint16_t classID, uint16_t typeID, uint32_t serialNum, uint32_t baudRate)
        {
            std::vector<serial::PortInfo> devices_found = serial::list_ports();
            for (const auto &dev : devices_found)
            {
#ifndef _WIN32
                if (dev.port.find("USB") == std::string::npos && dev.port.find("ACM") == std::string::npos)
                    continue;
#endif
                bool hasError = false;
                std::cout << "Trying Port: " << dev.port << std::endl;
                try
                {
                    serial_device_ = new serial::Serial(dev.port, baudRate, serial::Timeout::simpleTimeout(200));
                    if (serial_device_->isOpen())
                    {
                        if (getDeviceInfo())
                        {
                            if (classID == device_description_.class_id && typeID == device_description_.type_id)
                            {
                                if (serialNum != kSerialNumberAny)
                                {
                                    if (serialNum == device_description_.serial)
                                        return true;
                                } else
                                    return true;
                            }
                        }
                        serial_device_->close();
                    }
                }
                catch (serial::IOException &e)
                {
                    std::cout << "Exception: " << e.what() << std::endl;
                    hasError = true;
                }
                if (!hasError)
                {
                    delete serial_device_;
                }
                serial_device_ = nullptr;
            }
            return false;
        }

        bool SerialDeviceHost::getDeviceInfo()
        {
            data_.clear();
            has_device_info_ = false;
            for (int x = 0; x < 4; x++)
            {
                sendPacket(SD_COMMAND_GET_INFO);
#ifdef _WIN32
                Sleep(50);
#else
                usleep(50000);
#endif
                update();
                if (has_device_info_)
                    return true;
            }
            return false;
        }

        bool SerialDeviceHost::isOpen() const
        {
            if (serial_device_ != nullptr)
                return serial_device_->isOpen();

            return device_open_;
        }

        SerialDeviceHost::~SerialDeviceHost() = default;

        uint16_t SerialDeviceHost::getClassID() const
        {
            return device_description_.class_id;
        }

        uint16_t SerialDeviceHost::getTypeID() const
        {
            return device_description_.type_id;
        }

        uint32_t SerialDeviceHost::getSerial() const
        {
            return device_description_.serial;
        }

        uint8_t SerialDeviceHost::getVersionMajor() const
        {
            return device_description_.version_major;
        }

        uint8_t SerialDeviceHost::getVersionMinor() const
        {
            return device_description_.version_minor;
        }

        uint8_t SerialDeviceHost::getVersionRev() const
        {
            return device_description_.version_revision;
        }

        std::string SerialDeviceHost::getName() const
        {
            return device_description_.name;
        }

        std::string SerialDeviceHost::getInfo() const
        {
            return device_description_.info;
        }

        std::string SerialDeviceHost::getPortName() const
        {
            return serial_device_->getPort();
        }

        bool SerialDeviceHost::isFound() const
        {
            return found_device_;
        }

        void SerialDeviceHost::update()
        {
            if (serial_device_->available())
            {
                uint8_t data;
                serial_device_->read(&data, 1);
                if (data == header_byte_)
                {
                    serial_device_->read(&data, 1);
                    if (data == packet_id_)
                    {
                        in_packet_.clear();
                        uint8_t b1, b2;
                        serial_device_->read(&b1, 1);
                        serial_device_->read(&b2, 1);
                        uint16_t pSize = (b1 << 8) + b2;
                        uint8_t cmd;
                        serial_device_->read(&cmd, 1);
                        serial_device_->read(in_packet_, pSize - 4);
                        serial_device_->read(&b1, 1);
                        serial_device_->read(&b2, 1);
                        uint8_t stopByte;
                        serial_device_->read(&stopByte, 1);
                        uint16_t packCRC16 = (b1 << 8) + b2;

                        // Check CRC
                        uint16_t calcCRC16 = crc_.calculate(in_packet_.data(), 0, in_packet_.size());
                        if (calcCRC16 != packCRC16)
                        {
                            data_available_ = false;
                            data_error_ = true;
                            return;
                        }
                        data_error_ = false;
                        if (stopByte == stop_byte_)
                        {
                            if (cmd == (uint8_t) SD_COMMAND_SEND_DATA)
                            {
                                if (!in_packet_.empty())
                                {
                                    deserialize(in_packet_);
                                    data_available_ = true;
                                    data_read_ = false;
                                    return;
                                }
                            } else if (cmd == (uint8_t) SD_COMMAND_SEND_INFO)
                            {
                                if (!in_packet_.empty())
                                {
                                    deserialize(in_packet_);
                                    device_description_.class_id = get<uint16_t>("class");
                                    device_description_.type_id = get<uint16_t>("type");
                                    device_description_.serial = get<uint32_t>("serial");
                                    device_description_.version_major = get<uint8_t>("v1");
                                    device_description_.version_minor = get<uint8_t>("v2");
                                    device_description_.version_revision = get<uint8_t>("v3");
                                    device_description_.name = get<std::string>("name");
                                    device_description_.info = get<std::string>("info");
                                    has_device_info_ = true;
                                    data_available_ = false;
                                    return;
                                }
                            }
                        }
                    }
                }
            } else
            {
                if (data_read_)
                    data_available_ = false;
            }
        }

        void SerialDeviceHost::sendPacket(kSudCommandType cmd)
        {
            out_packet_.clear();
            uint16_t pSize = 0;
            cmd_ = cmd;
            if (cmd_ == SD_COMMAND_GET_INFO)
                has_device_info_ = false;
            out_packet_.emplace_back(header_byte_);
            out_packet_.emplace_back(packet_id_);
            out_packet_.emplace_back(0); // Place holder for packet size
            out_packet_.emplace_back(0); // Place holder for packet size
            out_packet_.emplace_back(cmd_);
            pSize++;
            pSize += serialize(out_packet_);
            out_packet_.emplace_back(0); // Place holder for CRC-16
            out_packet_.emplace_back(0); // Place holder for CRC-16
            out_packet_.emplace_back(stop_byte_);
            pSize += 3;
            out_packet_.at(2) = pSize >> 8;
            out_packet_.at(3) = pSize & 0xFF;

            // Calculate and insert CRC16
            uint16_t calcCRC16 = crc_.calculate(out_packet_.data(), 5, out_packet_.size() - 3);
            out_packet_.at(out_packet_.size() - 3) = calcCRC16 >> 8;
            out_packet_.at(out_packet_.size() - 2) = calcCRC16 & 0xFF;
            serial_device_->write(out_packet_);
        }

        bool SerialDeviceHost::available() const
        {
            return data_available_;
        }

        bool SerialDeviceHost::isError() const
        {
            return data_error_;
        }
    } // end namespace serial_device
} // end namespace rw
