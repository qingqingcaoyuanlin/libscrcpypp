//
// Created by ender on 25-2-5.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <iostream>
#include <utility>
#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace scc {
    class client {
    public:
        client(std::string addr, const std::uint16_t port) : addr(std::move(addr)), port(port) {
        }

        auto connect() {
            using boost::asio::ip::tcp;
            boost::asio::io_context io_context;

            tcp::resolver resolver(io_context);
            const auto endpoints = resolver.resolve(addr, std::to_string(port));

            tcp::socket socket(io_context);

            boost::asio::connect(socket, endpoints);

            std::array<char, 1> dummy_byte_buffer = {};
            try {
                socket.read_some(boost::asio::buffer(dummy_byte_buffer));
                if (dummy_byte_buffer[0] != 0x00) {
                    throw std::runtime_error(std::format("broken packet, expect 0x00 but got {:#x}.",
                                                         dummy_byte_buffer[0]));
                }
                std::cout << "successfully read dummy byte." << std::endl;
            } catch (std::exception &e) {
                std::cerr << "error reading dummy byte: " << e.what() << std::endl;
                return;
            }
            std::array<char, 64> device_name_buffer = {};
            socket.read_some(boost::asio::buffer(device_name_buffer));
            this->device_name = device_name_buffer.data();
            std::cout << "device name: " << device_name << std::endl;
            std::array<char, 4> size_buffer = {};
            socket.read_some(boost::asio::buffer(size_buffer));
            this->width = *reinterpret_cast<std::uint16_t *>(size_buffer.data());
            this->height = *reinterpret_cast<std::uint16_t *>(size_buffer.data() + 2);
            std::cout << "video stream working at resolution " << this->width << "x" << this->height << std::endl;
        }

        static auto start_scrcpy_server() {
        }

    private:
        std::string addr;
        std::uint16_t port;

        std::string device_name{};
        std::uint16_t height{0};
        std::uint16_t width{0};
    };
}
#endif //CLIENT_HPP
