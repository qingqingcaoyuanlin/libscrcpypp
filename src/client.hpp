//
// Created by ender on 25-2-5.
//

#ifndef SCRCPY_CLIENT_HPP
#define SCRCPY_CLIENT_HPP


#include <utility>
#include <queue>
#include <filesystem>
#include <ranges>
#include <boost/asio.hpp>
#include <boost/process.hpp>

namespace scrcpy {
    class client : public std::enable_shared_from_this<client> {
    public:
        client(std::string addr, std::uint16_t port);

        ~client();

        auto connect() -> void;

        auto start_recv() -> void;

        auto stop_recv() -> void;

        auto frame() -> std::vector<std::byte>;

        std::tuple<std::uint64_t, std::uint64_t> get_w_size();

        static auto read_forward(const std::filesystem::path &adb_bin) -> std::vector<std::array<std::string, 3> >;

        static auto forward_list_contains_tcp_port(
            const std::filesystem::path &adb_bin, std::uint16_t port) -> std::optional<std::string>;

        static auto list_dev_serials(const std::filesystem::path &adb_bin) -> std::vector<std::string>;

        auto get_codec() -> std::string {
            return this->codec;
        }

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

        boost::process::child server_c;

        std::atomic<bool> recv_enabled{false};

        std::shared_ptr<boost::asio::ip::tcp::socket> video_socket;

        std::mutex frame_mutex;
        std::queue<std::vector<std::byte> > frame_queue;
    };
}
#endif //SCRCPY_CLIENT_HPP
