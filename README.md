# libscrcpy++

[![C++](https://img.shields.io/badge/C++-23-blue)](https://en.cppreference.com/w/cpp/23)
[![License](https://img.shields.io/badge/License-MIT-blue)](LICENSE)

![Actions Workflow Windows](https://github.com/deniskovalchuk/ftp-client/actions/workflows/windows.yml/badge.svg)
![Actions Workflow Linux](https://github.com/deniskovalchuk/ftp-client/actions/workflows/linux.yml/badge.svg)
![Actions Workflow macOS](https://github.com/deniskovalchuk/ftp-client/actions/workflows/macos.yml/badge.svg)

A simple C++ implementation of scrcpy client build on C++23
and [Boost.Asio](https://www.boost.org/doc/libs/1_87_0/doc/html/boost_asio.html). Use this lib to capture video stream
from android devices with low-latency
and high-quality.

## before use

- Prepare an adb binary.
- Prepare a scrcpy-server jar and get its exact version.
- Make sure that your android devices are already connected to your host.

## codec support

- H.264 ✓
- H.265 ✕
- AV1 ✕

## example

```c++
#include "client.cpp"

int main(int argc, char *argv[]) {
    std::cout << "list of device serials:" << std::endl;
    for (const auto &s: scrcpy::client::list_dev_serials("adb")) {
        std::cout << s << std::endl;
    }

    const auto cli = scrcpy::client::create_shared("localhost", 1234);
    cli->deploy("adb", "scrcpy-server", "3.1", 1234);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // wait  1 sec for scrcpy server to start up
    cli->connect();
    cli->run_recv();
    auto codec = cli->get_codec();
    auto [w, h] = cli->video_size();
    std::printf("video size: [h:%lu,w:%lu]\n", w, h);
    for (const auto &frame: cli->frames()) {
        std::printf("pixel format:%s\n", av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->raw()->format)));
        std::printf("h:%d w:%d\n", frame->raw()->height, frame->raw()->width);
        auto mat = frame->mat();
        imwrite("test.png", *mat);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
```

Use cmake to find package and link static library.

```cmake

find_package(scrcpy++ REQUIRED)

target_link_libraries(example-target PUBLIC scrcpy::scrcpy++)

```

## build requirements

- Boost 1.80.0+
- C++ compiler that supports C++23 standard
- Windows or any unix-like system
- opencv 4.x
- ffmpeg libs

## docker

providing public docker env for building and
testing: [enderthecoder/libscrcpypp](https://hub.docker.com/repository/docker/enderthecoder/libscrcpypp)

```shell
docker pull enderthecoder/libscrcpypp:latest
```

## build && install

Use following commands to build and install this lib on your machine. Notice that install lib might require sudo
permission.

```shell
git clone https://github.com/EnderTheCoder/libscrcpypp
cd libscrcpypp
mkdir build
cd build
cmake ..
cmake --build .
cmake --install .
```

## download official windows && linux build

Click the following link and pick up an artifact:

[gitlab build site](https://git.ender.cool/EnderTheCoder/libscrcpypp/-/artifacts)

Official CI build for macOS is currently unavailable(I don't have a mac myself).