#include "client.hpp"
int main(int argc, char *argv[]) {
    auto cli = scc::client("localhost", 1234);
    cli.connect();
    cli.start_recv();
    std::mutex mtx;
    mtx.lock();
    mtx.lock();

    return 0;
}
