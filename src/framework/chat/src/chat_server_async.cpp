#include <chat_server_async.hpp>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port> ...\n";
        return 1;
    }

    net::io_context ioc;
    std::list<listener> servers;

    for (int i = 1; i < argc; ++i)
    {
        endpoint_t endpoint(tcp::v4(), std::atoi(argv[i]));
        servers.emplace_back(ioc, endpoint);
    }

    ioc.run();

    return 0;
}
