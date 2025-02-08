//
// Created by ender on 25-2-5.
//

#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <iostream>
#include <utility>
#include <queue>
#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace scc {
    class client : public std::enable_shared_from_this<client> {
    public:
        client(std::string addr, const std::uint16_t port) : addr(std::move(addr)), port(port) {
        }

        auto connect() {
            using boost::asio::ip::tcp;
            boost::asio::io_context io_context;

            tcp::resolver resolver(io_context);
            const auto endpoints = resolver.resolve(addr, std::to_string(port));

            this->video_socket = std::make_shared<tcp::socket>(io_context);
            boost::asio::connect(*this->video_socket, endpoints);

            std::array<char, 1> dummy_byte_buffer = {};
            try {
                this->video_socket->read_some(boost::asio::buffer(dummy_byte_buffer));
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
            this->video_socket->read_some(boost::asio::buffer(device_name_buffer));
            this->device_name = device_name_buffer.data();
            std::cout << "device name: " << device_name << std::endl;
            std::array<std::byte, 12> codec_meta_buffer = {};
            this->video_socket->read_some(boost::asio::buffer(codec_meta_buffer));
            this->codec_id = std::string{reinterpret_cast<char *>(codec_meta_buffer.data()), 4};
            std::reverse(codec_meta_buffer.begin() + 4, codec_meta_buffer.begin() + 8);
            std::reverse(codec_meta_buffer.begin() + 8, codec_meta_buffer.end());
            this->width = *reinterpret_cast<std::uint32_t *>(codec_meta_buffer.data() + 4);
            this->height = *reinterpret_cast<std::uint32_t *>(codec_meta_buffer.data() + 8);
            std::cout << "video stream working at resolution " << this->height << "x" << this->width << std::endl;
        }

        auto start_recv() {
            this->recv_enabled = true;
            std::thread t([this] {
                while (true) {
                    if (not recv_enabled) {
                        break;
                    }
                    std::vector<std::byte> frame_buffer;
                    frame_buffer.reserve(0x10000);

                    while (true) {
                        std::array<std::byte, 0x10000> net_buffer = {};

                        const auto size = this->video_socket->read_some(boost::asio::buffer(net_buffer));
                        frame_buffer.insert(frame_buffer.end(), net_buffer.begin(), net_buffer.begin() + size);
                        if (size < 0x10000) {
                            break;
                        }
                    }
                    std::cout << "frame received with size" << frame_buffer.size() << std::endl;

                    this->data_queue.emplace(std::move(frame_buffer));
                    if (this->data_queue.size() > 3) {
                        this->data_queue.pop();
                    }
                }
            });
            t.detach();
        }

        auto stop_recv() {
            this->recv_enabled = false;
        }

        static auto start_scrcpy_server() {
            //adb shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 3.1 tunnel_forward=true cleanup=false audio=false control=false max_size=1920
        }

    private:
        std::string addr;
        std::uint16_t port;

        std::string device_name{};
        std::string codec_id{};
        std::uint32_t height{0};
        std::uint32_t width{0};

        std::atomic<bool> recv_enabled{false};

        std::shared_ptr<boost::asio::ip::tcp::socket> video_socket;

        std::queue<std::vector<std::byte> > data_queue;
    };
}
#endif //CLIENT_HPP
