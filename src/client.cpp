//
// Created by ender on 25-2-15.
//
#include <client.hpp>

namespace scrcpy {
    auto client::create_shared(const std::string &addr, std::uint16_t port) -> std::shared_ptr<client> {
        return std::make_shared<client>(addr, port);
    }

    client::client(std::string addr, const std::uint16_t port): addr(std::move(addr)), port(port) {
    }

    client::~client() {
        stop_recv();
        if (recv_handle.joinable()) recv_handle.join();
        if (config_packet != nullptr) {
            av_packet_free(&config_packet);
        }
        if (video_socket != nullptr and video_socket->is_open()) {
            video_socket->cancel();
            video_socket->close();
        }

        if (server_c.running()) {
            server_c.terminate();
            server_c.wait();
        }
    }

    auto client::connect() -> void {
        using boost::asio::ip::tcp;
        io_context = std::make_shared<boost::asio::io_context>();

        tcp::resolver resolver(*io_context);
        const auto endpoints = resolver.resolve(addr, std::to_string(port));

        this->video_socket = std::make_shared<tcp::socket>(*io_context);
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
        this->codec = std::string{reinterpret_cast<char *>(codec_meta_buffer.data()), 4};
        std::reverse(codec_meta_buffer.begin() + 4, codec_meta_buffer.begin() + 8);
        std::reverse(codec_meta_buffer.begin() + 8, codec_meta_buffer.end());
        this->width = *reinterpret_cast<std::uint32_t *>(codec_meta_buffer.data() + 4);
        this->height = *reinterpret_cast<std::uint32_t *>(codec_meta_buffer.data() + 8);\
        std::cout << "video stream codec: " << this->codec << std::endl;
        std::cout << "video stream working at resolution: " << this->height << "x" << this->width << std::endl;
    }

    auto client::run_recv() -> void {
        recv_enabled = true;
        while (true) {
            if (not recv_enabled) {
                break;
            }
            std::array<std::uint8_t, 12> frame_header_buffer{};

            const auto frame_header_size = this->video_socket->read_some(boost::asio::buffer(frame_header_buffer));
            if (frame_header_size != frame_header_buffer.size()) {
                throw std::runtime_error(
                    std::format("broken frame header, expect {:#x}. got {:#x}",
                                frame_header_buffer.size(), frame_header_size));
            }
            const bool config_flag = frame_header_buffer.at(0) >> 7 & 0x01;
            const bool keyframe_flag = frame_header_buffer.at(0) >> 6 & 0x01;
            std::reverse(frame_header_buffer.begin(), frame_header_buffer.begin() + 8);
            frame_header_buffer.at(7) <<= 2;
            const auto pts = *reinterpret_cast<std::uint64_t *>(frame_header_buffer.data());
            std::reverse(frame_header_buffer.begin() + 8, frame_header_buffer.end());
            const auto packet_size = *reinterpret_cast<std::uint32_t *>(frame_header_buffer.data() + 8);


            AVPacket *packet = av_packet_alloc();
            if (av_new_packet(packet, static_cast<std::int32_t>(packet_size))) {
                throw std::runtime_error("failed to allocate packet memory: ");
            }
            const auto frame_size = this->video_socket->read_some(boost::asio::buffer(packet->data, packet_size));
            packet->size = static_cast<std::int32_t>(packet_size);

            if (frame_size != packet_size) {
                av_packet_free(&packet);
                if (this->config_packet != nullptr) {
                    av_packet_free(&this->config_packet);
                }
                std::cerr << "end of video stream" << std::endl;
                break;
            }
            if (config_flag) {
                packet->pts = AV_NOPTS_VALUE;
            } else {
                packet->pts = static_cast<std::int64_t>(pts);
            }

            if (keyframe_flag) {
                packet->flags |= AV_PKT_FLAG_KEY;
            }

            packet->dts = packet->pts;
            if (config_flag) {
                config_packet = packet;
            } else if (config_packet != nullptr) {
                if (av_grow_packet(packet, config_packet->size)) {
                    throw std::runtime_error("failed to grow packet");
                }
                memmove(packet->data + config_packet->size, packet->data, packet->size);
                memcpy(packet->data, config_packet->data, config_packet->size);
                // packet->size += config_packet->size;
                av_packet_free(&config_packet);
                config_packet = nullptr;
            }

            const auto frames = decoder.decode(packet);
            if (frames.empty()) {
                continue;
            }

            if (this->consumer.has_value()) {
                for (const auto &frame : frames) {
                    this->consumer.value()(frame);
                }
                continue;
            }

            frame_mutex.lock();
            frame_queue.insert(frame_queue.end(), frames.begin(), frames.end());
            frame_mutex.unlock();
        }
    }

    auto client::start_recv() -> void {
        recv_handle = std::thread([t = shared_from_this()] {
            t->run_recv();
        });
    }

    auto client::stop_recv() -> void {
        this->recv_enabled = false;
    }

    auto client::is_recv_enabled() -> bool {
        return this->recv_enabled;
    }

    auto client::set_frame_consumer(std::function<void(std::shared_ptr<frame>)> consumer) -> void {
        this->consumer = consumer;
    }

    // auto client::start_decode() -> void {
    //     parse_enabled = true;
    //     std::thread t([this] {
    //         while (true) {
    //             if (not parse_enabled) {
    //                 break;
    //             }
    //             std::unique_lock lock(raw_mutex);
    //             decode_cv.wait(lock, [this] {
    //                 return !raw_queue.empty();
    //             });
    //             std::vector<std::byte> decode_buffer;
    //             decode_buffer.insert(decode_buffer.end(), raw_queue.begin(), raw_queue.end());
    //             raw_queue.clear();
    //             lock.unlock();
    //             const auto frames = decoder.decode(std::span{decode_buffer});
    //             if (frames.empty()) {
    //                 continue;
    //             }
    //             std::lock_guard guard(frame_mutex);
    //             frame_queue.insert(frame_queue.end(), frames.begin(), frames.end());
    //         }
    //     });
    //     t.detach();
    // }
    //
    // auto client::stop_decode() -> void {
    //     parse_enabled = false;
    // }

    auto client::frames() -> std::vector<std::shared_ptr<frame>> {
        frame_mutex.lock();
        if (frame_queue.empty()) {
            return {};
        }
        std::vector<std::shared_ptr<frame>> frames = {};
        frames.insert(frames.end(), frame_queue.begin(), frame_queue.end());
        frame_queue.clear();
        frame_mutex.unlock();
        return frames;
    }

    auto client::video_size() -> std::tuple<std::uint64_t, std::uint64_t> {
        return {width, height};
    }

    auto client::read_forward(
        const std::filesystem::path &adb_bin) -> std::vector<std::array<std::string, 3> > {
        using namespace boost::process;
        ipstream out_stream;
        using std::operator""sv;
        child list_c(std::format("{} forward --list", adb_bin.string()), std_out > out_stream);
        std::vector<std::array<std::string, 3> > forward_list;
        list_c.wait();
        for (std::string line; out_stream && std::getline(out_stream, line) && !line.empty();) {
            auto item = std::array<std::string, 3>{};
            for (const auto [idx, part]: std::views::split(line, " "sv) | std::views::enumerate) {
                item.at(idx) = std::string_view(part);
            }
            forward_list.emplace_back(item);
        }
        return forward_list;
    }

    std::optional<std::string> client::forward_list_contains_tcp_port(
        const std::filesystem::path &adb_bin,
        const std::uint16_t port) {
        for (const auto &[serial, local, remote]: read_forward(adb_bin)) {
            if (local.contains(std::format("tcp:{}", port))) {
                return serial;
            }
        }
        return std::nullopt;
    }

    auto client::list_dev_serials(const std::filesystem::path &adb_bin) -> std::vector<std::string> {
        using namespace boost::process;
        ipstream out_stream;
        using std::operator""sv;
        child list_c(std::format("{} devices", adb_bin.string()), std_out > out_stream);
        auto sig_start = false;
        std::vector<std::string> serials;
        list_c.wait();
        for (std::string line; out_stream && std::getline(out_stream, line) && !line.empty();) {
            if (sig_start) {
                for (const auto [s_begin, s_end]: std::views::split(line, "\t"sv)) {
                    auto serial = std::string_view(s_begin, s_end);
                    serials.emplace_back(serial);
                    break;
                }
            } else if (line == "List of devices attached") {
                sig_start = true;
            }
        }
        return serials;
    }

    auto client::deploy(const std::filesystem::path &adb_bin,
                        const std::filesystem::path &scrcpy_jar_bin,
                        const std::string &scrcpy_server_version, const std::uint16_t port,
                        const std::optional<std::string> &device_serial) -> void {
        //adb shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 3.1 tunnel_forward=true cleanup=false audio=false control=false max_size=1920
        using namespace boost::process;
        auto adb_exec = adb_bin.string();
        std::string serial;
        if (device_serial.has_value()) {
            adb_exec += " -s " + device_serial.value();
            serial = device_serial.value();
        } else {
            ipstream out_stream;
            auto serial_c = child(std::format("{} get-serialno", adb_exec), std_out > out_stream);
            serial_c.wait();
            if (serial_c.exit_code() != 0) {
                throw std::runtime_error("failed to get adb device serialno");
            }
            for (std::string line; out_stream && std::getline(out_stream, line) && !line.empty();) {
                serial = line;
                break; // read first line only
            }
        }
        auto upload_cmd = std::format("{} push {} /sdcard/scrcpy-server.jar", adb_exec, scrcpy_jar_bin.string());
        auto forward_cmd = std::format("{} forward tcp:{} localabstract:scrcpy", adb_exec, port);
        auto exec_cmd = std::format(
            "{} shell CLASSPATH=/sdcard/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server"
            " {} tunnel_forward=true cleanup=true audio=false control=false",
            adb_exec, scrcpy_server_version
        );

        child upload_c(upload_cmd);
        upload_c.wait();
        if (upload_c.exit_code() != 0) {
            throw std::runtime_error("error uploading scrcpy server jar");
        }

        if (const auto existing_serial = forward_list_contains_tcp_port(adb_bin, port);
            existing_serial.has_value()) {
            if (existing_serial.value() != serial) {
                throw std::runtime_error(
                    std::format(
                        "another adb device[serial={}] is forwarding on this port[{}]",
                        existing_serial.value(), port)
                );
            }
        } else {
            child forward_c(forward_cmd);
            forward_c.wait();
            if (forward_c.exit_code() != 0) {
                throw std::runtime_error("error forwarding scrcpy to local tcp port");
            }
        }

        server_c = child{exec_cmd, std_out > server_out_stream};
    }
}
