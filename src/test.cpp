#include "client.hpp"

int main(int argc, char *argv[]) {
    auto cli = scc::client("localhost", 1234);
    cli.deploy("adb", "scrcpy-server", "3.1", 1234);
    std::this_thread::sleep_for(std::chrono::seconds(1)); // wait  1 sec for scrcpy server to start up
    cli.connect();
    cli.start_recv();
    auto frame = cli.frame();
    // do the codec stuff
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
