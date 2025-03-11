//
// Created by ender on 25-3-11.
//

#ifndef CONTROL_MSG_HPP
#define CONTROL_MSG_HPP
#include <cstdint>

namespace scrcpy {
    namespace pointer_id {
        constexpr std::uint64_t MOUSE = -1;
        constexpr std::uint64_t GENERIC_FINGER = -2;
        constexpr std::uint64_t VIRTUAL_FINGER = -3;
    }

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

    class control_msg {
    };
}


#endif //CONTROL_MSG_HPP
