#include "client.cpp"

int main(int argc, char *argv[]) {
    std::cout << "list of device serials:" << std::endl;
    for (const auto &s: scrcpy::client::list_dev_serials("adb")) {
        std::cout << s << std::endl;
    }

    const auto cli = scrcpy::client::create_shared("localhost", 1234);
    cli->deploy("adb", "scrcpy-server", "3.1", 1234);
    std::this_thread::sleep_for(std::chrono::seconds(1));
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
