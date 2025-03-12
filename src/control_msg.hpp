//
// Created by ender on 25-3-11.
//

#ifndef CONTROL_MSG_HPP
#define CONTROL_MSG_HPP
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <boost/asio/detail/socket_ops.hpp>

namespace scrcpy {
    namespace pointer_id {
        constexpr std::uint64_t MOUSE = -1;
        constexpr std::uint64_t GENERIC_FINGER = -2;
        constexpr std::uint64_t VIRTUAL_FINGER = -3;
    }

    enum class android_keyevent_action {
        /** The key has been pressed down. */
        AKEY_EVENT_ACTION_DOWN = 0,

        /** The key has been released. */
        AKEY_EVENT_ACTION_UP = 1,

        /**
         * Multiple duplicate key events have occurred in a row, or a
         * complex string is being delivered.  The repeat_count property
         * of the key event contains the number of times the given key
         * code should be executed.
         */
        AKEY_EVENT_ACTION_MULTIPLE = 2
    };

    enum class control_msg_type {
        SC_CONTROL_MSG_TYPE_INJECT_KEYCODE,
        SC_CONTROL_MSG_TYPE_INJECT_TEXT,
        SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
        SC_CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
        SC_CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
        SC_CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
        SC_CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL,
        SC_CONTROL_MSG_TYPE_COLLAPSE_PANELS,
        SC_CONTROL_MSG_TYPE_GET_CLIPBOARD,
        SC_CONTROL_MSG_TYPE_SET_CLIPBOARD,
        SC_CONTROL_MSG_TYPE_SET_DISPLAY_POWER,
        SC_CONTROL_MSG_TYPE_ROTATE_DEVICE,
        SC_CONTROL_MSG_TYPE_UHID_CREATE,
        SC_CONTROL_MSG_TYPE_UHID_INPUT,
        SC_CONTROL_MSG_TYPE_UHID_DESTROY,
        SC_CONTROL_MSG_TYPE_OPEN_HARD_KEYBOARD_SETTINGS,
        SC_CONTROL_MSG_TYPE_START_APP,
        SC_CONTROL_MSG_TYPE_RESET_VIDEO,
    };

    enum class copy_key {
        SC_COPY_KEY_NONE,
        SC_COPY_KEY_COPY,
        SC_COPY_KEY_CUT,
    };

    enum class android_motionevent_buttons {
        AMOTION_EVENT_BUTTON_NONE = 0,
        /** primary */
        AMOTION_EVENT_BUTTON_PRIMARY = 1 << 0,
        /** secondary */
        AMOTION_EVENT_BUTTON_SECONDARY = 1 << 1,
        /** tertiary */
        AMOTION_EVENT_BUTTON_TERTIARY = 1 << 2,
        /** back */
        AMOTION_EVENT_BUTTON_BACK = 1 << 3,
        /** forward */
        AMOTION_EVENT_BUTTON_FORWARD = 1 << 4,
        AMOTION_EVENT_BUTTON_STYLUS_PRIMARY = 1 << 5,
        AMOTION_EVENT_BUTTON_STYLUS_SECONDARY = 1 << 6,
    };

    class dtype {
    public:
        virtual ~dtype() = default;

        /**
         * data -> bytes
         * @return byte sequence
         */
        virtual auto serialize() -> std::vector<std::byte> = 0;
    };

    class sizable {
    public:
        virtual ~sizable() = default;

        virtual auto size() -> std::size_t = 0;
    };

    class fixed_size : public sizable {
    };

    template<typename INT_TYPE, std::size_t BYTE_SIZE = sizeof(INT_TYPE)>
    class abs_int_t : public dtype, public fixed_size {
        static_assert(std::is_integral_v<INT_TYPE>);

    public:
        explicit abs_int_t(const INT_TYPE value) : value(value) {
        }

        explicit operator INT_TYPE() const {
            return value;
        }

        auto serialize() -> std::vector<std::byte> override {
            auto arr_ptr = reinterpret_cast<std::array<std::byte, BYTE_SIZE> *>(&value);
            std::vector<std::byte> buffer{BYTE_SIZE};
            std::reverse_copy(arr_ptr->begin(), arr_ptr->end(), buffer.begin());
            return buffer;
        }

        auto size() -> std::size_t override {
            return BYTE_SIZE;
        }

        auto val() {
            return value;
        }

    private:
        INT_TYPE value{0};
    };

    template<typename FLOAT_TYPE, std::size_t BYTE_SIZE = sizeof(FLOAT_TYPE)>
    class abs_float_t final : public dtype, public fixed_size {
        static_assert(std::is_floating_point_v<FLOAT_TYPE>);

    public:
        explicit abs_float_t(const FLOAT_TYPE value) : value(value) {
        }

        explicit operator FLOAT_TYPE() const {
            return value;
        }

        auto serialize() -> std::vector<std::byte> override {
            auto arr_ptr = reinterpret_cast<std::array<std::byte, BYTE_SIZE> *>(&value);
            std::vector<std::byte> buffer{BYTE_SIZE};
            std::reverse_copy(arr_ptr->begin(), arr_ptr->end(), buffer.begin());
            return buffer;
        }

        auto size() -> std::size_t override {
            return BYTE_SIZE;
        }

        auto val() {
            return value;
        }

    private:
        FLOAT_TYPE value{0.0};
    };

    template<typename ENUM_TYPE>
    class abs_enum_t final : public abs_int_t<std::uint8_t> {
    public:
        explicit abs_enum_t(ENUM_TYPE value)
            : abs_int_t(static_cast<std::uint8_t>(value)) {
        }
    };

    class control_msg {
    public:
        virtual ~control_msg() = 0;

        [[nodiscard]] virtual auto buf_size() const -> std::size_t = 0;

        virtual auto serialize() -> std::vector<std::byte> = 0;

        virtual auto join_buf(const std::vector<std::byte> &buf) -> void;

        virtual auto init_buf() -> std::vector<std::byte>;

    private:
        std::vector<std::byte>::iterator buf_it;
    };

    class single_byte_msg final : public control_msg {
    public:

        explicit single_byte_msg(control_msg_type type);

        [[nodiscard]] auto buf_size() const -> std::size_t override;

        auto serialize() -> std::vector<std::byte> override;

    private:
        std::optional<abs_enum_t<control_msg_type> > msg_type;
    };

    class position_t final : public fixed_size, public dtype {
    public:
        position_t(std::int32_t x, std::int32_t y, std::uint16_t width, std::uint16_t height);

        ~position_t() override;

        auto size() -> std::size_t override;

        auto serialize() -> std::vector<std::byte> override;

    private:
        abs_int_t<std::int32_t> x;
        abs_int_t<std::int32_t> y;

        abs_int_t<std::uint16_t> width;
        abs_int_t<std::uint16_t> height;
    };

    class mouse_msg final : public control_msg {
    public:
        [[nodiscard]] auto buf_size() const -> std::size_t override;

        auto serialize() -> std::vector<std::byte> override;

        std::optional<abs_enum_t<control_msg_type> > msg_type =
                abs_enum_t{control_msg_type::SC_CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT};
        std::optional<abs_enum_t<android_keyevent_action> > action;
        std::optional<abs_int_t<std::uint64_t> > pointer_id;
        std::optional<position_t> position;
        std::optional<abs_float_t<float> > pressure;
        std::optional<abs_enum_t<android_motionevent_buttons> > action_button;
        std::optional<abs_enum_t<android_motionevent_buttons> > buttons;
    };
}


#endif //CONTROL_MSG_HPP
