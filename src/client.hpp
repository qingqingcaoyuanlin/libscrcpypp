//
// Created by ender on 25-2-5.
//

#ifndef SCRCPY_CLIENT_HPP
#define SCRCPY_CLIENT_HPP


#include <utility>
#include <deque>
#include <filesystem>
#include <ranges>
#include <bitset>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include "decoder.hpp"
#include "frame.hpp"

namespace scrcpy {
    class client : public std::enable_shared_from_this<client> {
    public:
        static auto create_shared(std::string_view addr, std::uint16_t port) -> std::shared_ptr<client>;

        client(std::string_view addr, std::uint16_t port);

        ~client();

        auto get_port() const -> std::uint16_t;

        auto get_addr() const -> std::string_view;

        auto connect() -> void;

        auto start_recv() -> void;

        auto stop_recv() -> void;

        auto is_recv_enabled() -> bool;

        auto set_frame_consumer(std::function<void(std::shared_ptr<frame>)> consumer) -> void;

        // auto start_decode() -> void;
        //
        // auto stop_decode() -> void;

        auto frames() -> std::vector<std::shared_ptr<frame> >;

        auto video_size() -> std::tuple<std::uint64_t, std::uint64_t>;

        static auto read_forward(const std::filesystem::path &adb_bin) -> std::vector<std::array<std::string, 3> >;

        static auto forward_list_contains_tcp_port(
            const std::filesystem::path &adb_bin, std::uint16_t port) -> std::optional<std::string>;

        static auto list_dev_serials(const std::filesystem::path &adb_bin) -> std::vector<std::string>;

        auto get_codec() -> std::string {
            return this->codec;
        }

        auto run_recv() -> void;

        auto deploy(const std::filesystem::path &adb_bin,
                    const std::filesystem::path &scrcpy_jar_bin,
                    const std::string &scrcpy_server_version,
                    std::uint16_t port,
                    const std::optional<std::string> &device_serial = std::nullopt) -> void;

    private:
        std::string addr;
        std::uint16_t port;

        std::string device_name{};
        std::string codec{};
        std::uint32_t height{0};
        std::uint32_t width{0};

        std::thread recv_handle;

        boost::process::child server_c;
        boost::process::ipstream server_out_stream;

        std::atomic<bool> recv_enabled{false};
        std::atomic<bool> parse_enabled{false};

        std::shared_ptr<boost::asio::ip::tcp::socket> video_socket;
        std::shared_ptr<boost::asio::io_context> io_context;

        // std::mutex raw_mutex;
        std::mutex frame_mutex;

        // std::condition_variable decode_cv;

        // std::deque<std::byte> raw_queue;
        std::deque<std::shared_ptr<frame> > frame_queue;

        h264_decoder decoder;
        AVPacket *config_packet = nullptr;

        std::optional<std::function<void(std::shared_ptr<frame>)> > consumer;
    };
}
#endif //SCRCPY_CLIENT_HPP
