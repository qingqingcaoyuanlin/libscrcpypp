//  ______   ___   __    ______   ______   ______   _________  ___   ___   ______   ______   ______   ______   ______   ______
// /_____/\ /__/\ /__/\ /_____/\ /_____/\ /_____/\ /________/\/__/\ /__/\ /_____/\ /_____/\ /_____/\ /_____/\ /_____/\ /_____/\
// \::::_\/_\::\_\\  \ \\:::_ \ \\::::_\/_\:::_ \ \\__.::.__\/\::\ \\  \ \\::::_\/_\:::__\/ \:::_ \ \\:::_ \ \\::::_\/_\:::_ \ \
//  \:\/___/\\:. `-\  \ \\:\ \ \ \\:\/___/\\:(_) ) )_ \::\ \   \::\/_\ .\ \\:\/___/\\:\ \  __\:\ \ \ \\:\ \ \ \\:\/___/\\:(_) ) )_
//   \::___\/_\:. _    \ \\:\ \ \ \\::___\/_\: __ `\ \ \::\ \   \:: ___::\ \\::___\/_\:\ \/_/\\:\ \ \ \\:\ \ \ \\::___\/_\: __ `\ \
//    \:\____/\\. \`-\  \ \\:\/.:| |\:\____/\\ \ `\ \ \ \::\ \   \: \ \\::\ \\:\____/\\:\_\ \ \\:\_\ \ \\:\/.:| |\:\____/\\ \ `\ \ \
//     \_____\/ \__\/ \__\/ \____/_/ \_____\/ \_\/ \_\/  \__\/    \__\/ \::\/ \_____\/ \_____\/ \_____\/ \____/_/ \_____\/ \_\/ \_\/
//
// Copyright (C) 2025 EnderTheCoder ggameinvader@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by ender on 25-3-12.
//

#include <client.hpp>
using namespace scrcpy;
auto main() -> int {
    const auto cli = client::create_shared("localhost", 1234);
    cli->deploy("adb", "scrcpy-server", "3.1", 1234, std::nullopt, 720);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cli->connect();
    auto msg = std::make_shared<mouse_msg>();
    msg->action = abs_enum_t{android_keyevent_action::AKEY_EVENT_ACTION_DOWN};
    msg->pointer_id = abs_int_t{pointer_id::MOUSE};
    auto position = position_t(200, 200, 720, 336);
    msg->position = position;
    msg->pressure = abs_float_t{1.0f};
    msg->action_button = abs_enum_t{android_motionevent_buttons::AMOTION_EVENT_BUTTON_NONE};
    msg->buttons = abs_enum_t{android_motionevent_buttons::AMOTION_EVENT_BUTTON_NONE};
    cli->send_control_msg(msg);
    return 0;
}
