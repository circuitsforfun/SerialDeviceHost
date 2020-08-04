
#include "SerialDeviceDiscovery.hpp"

namespace rw
{
    namespace serial_device
    {
        SerialDeviceDiscovery::SerialDeviceDiscovery()
        {
            found_device_ = false;
            device_open_ = false;
            data_available_ = false;
            data_error_ = false;
            header_byte_ = 0xAA;
            packet_id_ = 0xBB;
            stop_byte_ = 0xDD;
            serial_device_ = nullptr;
        }

        bool SerialDeviceDiscovery::getDeviceInfo()
        {
            data_.clear();
            has_device_info_ = false;
            for (int x = 0; x < 4; x++)
            {
                serial_device_->flushInput();
                serial_device_->flushOutput();
#ifdef _WIN32
                Sleep(100);
#else
                usleep(100000);
#endif
                sendPacket(SD_COMMAND_GET_INFO);
#ifdef _WIN32
                Sleep(50);
#else
                usleep(50000);
#endif
                update();
                return has_device_info_;
            }
            return false;
        }

        void SerialDeviceDiscovery::update()
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

        void SerialDeviceDiscovery::sendPacket(kSudCommandType cmd)
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

        std::vector<deviceList> SerialDeviceDiscovery::getDeviceList()
        {
            std::vector<uint32_t> baudRateCheck = {4800, 9600, 19200, 38400,
                                                   57600, 115200, 230400, 256000,
                                                   250000, 460800, 500000, 921600}; // Most Common Baud Rates to Check
            std::vector<deviceList> sdl;
            std::vector<serial::PortInfo> devices_found = serial::list_ports();
            //std::cout << "Found: " << devices_found.size() << " port(s)" << std::endl;
            for (auto dev : devices_found)
            {
#ifndef _WIN32
                if (dev.port.find("USB") == std::string::npos && dev.port.find("ACM") == std::string::npos)
                    continue;
#endif
                bool hasError = false;
                //std::cout << "Checking Port: " << dev.port << std::endl;
                try
                {
                    serial_device_ = new serial::Serial(dev.port, baudRateCheck.at(0), serial::Timeout::simpleTimeout(500));
                    if (serial_device_->isOpen())
                    {
                        //std::cout << "Port Opened" << std::endl;
                        for (int i = 0; i < baudRateCheck.size(); i++)
                        {
                            //std::cout << "Trying Baud Rate: " << " Baud: " << baudRateCheck.at(i) << std::endl;
                            serial_device_->setBaudrate(baudRateCheck.at(i));
                            if (getDeviceInfo())
                            {
                                //std::cout << "Got Device Info" << std::endl;
                                deviceList detected;
                                detected.baudRate = baudRateCheck.at(i);
                                detected.commPort = dev.port;
                                detected.class_id = device_description_.class_id;
                                detected.type_id = device_description_.type_id;
                                detected.serial = device_description_.serial;
                                detected.version_major = device_description_.version_major;
                                detected.version_minor = device_description_.version_minor;
                                detected.version_revision = device_description_.version_revision;
                                detected.name = device_description_.name;
                                detected.info = device_description_.info;
                                sdl.emplace_back(detected);
                                serial_device_->close();
                                break;
                            }
                        }
                        //std::cout << "Closing Port" << std::endl;
                        serial_device_->close();
                    } else
                    {
                        //std::cout << "Couldn't Open Port" << std::endl;
                    }
                }
                catch (serial::IOException &e)
                {
                    //std::cout << "exception: " << e.what() << std::endl;
                    hasError = true;
                }
                if (!hasError)
                {
                    //std::cout << "Delete Instance" << std::endl;
                    delete serial_device_;
                }
                serial_device_ = nullptr;
            }
            return sdl;
        }
    }
}