#include <net_client_async.hpp>

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        std::cerr << "Usage:   " << argv[0] << " <host> <port> <text> <service>\n"
                  << "Example: " << argv[0] << " 127.0.0.1 100 \"template <typename T>\" [1-3]\n";
        return 1;
    }

    auto const host = argv[1];
    auto const port = argv[2];
    auto const text = argv[3];
    int srv = std::stoi(argv[4]);

    net::io_context ioc;

    std::make_shared<session>(ioc, srv)->run(host, port, text);
    ioc.run();

    return 0;
}
