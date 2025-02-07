#include "client.hpp"
int main(int argc, char *argv[]) {
    auto cli = scc::client("localhost", 1234);
    cli.connect();
    return 0;
}
