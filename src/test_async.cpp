//
// Created by ender on 25-2-17.
//
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
    auto codec = cli->get_codec();
    auto [w, h] = cli->video_size();
    std::printf("video size: [h:%lu,w:%lu]\n", w, h);
    cli->set_frame_consumer([](const auto &frame) {
        std::printf("pixel format:%s\n", av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->raw()->format)));
        std::printf("h:%d w:%d\n", frame->raw()->height, frame->raw()->width);
        const auto t1 = std::chrono::high_resolution_clock::now();
        auto mat = frame->mat();
        const auto t2 = std::chrono::high_resolution_clock::now();
        const auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
        std::printf("pix trans fmt time: %ld µs\n", time_span.count());
    });
    cli->run_recv();
    return 0;
}
