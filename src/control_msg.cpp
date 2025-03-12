//
// Created by ender on 25-3-11.
//

#include "control_msg.hpp"

namespace scrcpy {
    auto control_msg::join_buf(const std::vector<std::byte> &buf) -> void {
        std::copy_n(buf.data(), buf.size(), buf_it);
        buf_it += static_cast<std::int64_t>(buf.size());
    }

    auto control_msg::init_buf() -> std::vector<std::byte> {
        std::vector<std::byte> buf(this->buf_size());
        this->buf_it = buf.begin();
        return buf;
    }

    position_t::position_t(const std::int32_t x, const std::int32_t y, const std::uint16_t width,
                           const std::uint16_t height)
        : x(x), y(y), width(width), height(height) {
    }

    position_t::~position_t() = default;

    auto position_t::size() -> std::size_t {
        return 12;
    }

    auto position_t::serialize() -> std::vector<std::byte> {
        std::vector<std::byte> buf;
        buf.resize(this->size());
        std::copy_n(x.serialize().begin(), 4, buf.begin());
        std::copy_n(y.serialize().begin(), 4, buf.begin() + 4);
        std::copy_n(width.serialize().begin(), 2, buf.begin() + 8);
        std::copy_n(height.serialize().begin(), 2, buf.begin() + 10);
        return buf;
    }

    std::size_t mouse_msg::buf_size() const {
        return 32;
    }

    std::vector<std::uint8_t> mouse_msg::serialize() {
        std::vector<std::uint8_t> buf;
        buf.resize(buf_size());
        this->join_buf(msg_type.value().serialize());
        this->join_buf(action.value().serialize());
        this->join_buf(pointer_id.value().serialize());
        this->join_buf(pressure.value().serialize());
        this->join_buf(action_button.value().serialize());
        this->join_buf(buttons.value().serialize());
        return buf;
    }
}
