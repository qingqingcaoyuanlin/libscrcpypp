#include "client.hpp"
int main(int argc, char *argv[]) {
    auto cli = scc::client("localhost", 1234);
    cli.deploy("adb", "scrcpy-server.jar", "3.1", 1234);
    cli.connect();
    cli.start_recv();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    return 0;
}
