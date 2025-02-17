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
    auto frames = cli->frames();
    auto codec = cli->get_codec();
    auto [w, h] = cli->get_w_size();
    for (auto frame: frames) {
        std::printf("pixel format:%s\n", av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)));
        std::printf("h:%d w:%d\n", frame->height, frame->width);
        auto mat = scrcpy::h264_decoder::avframe_to_mat(frame);
        imshow("image", mat);
        av_frame_free(&frame);
    }
    cli->stop_recv();
    return 0;
}
