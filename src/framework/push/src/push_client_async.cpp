#include <push_client_async.hpp>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage:   " << argv[0] << " <host> <port>\n"
                  << "Example: " << argv[0] << " 127.0.0.1 8080\n";
        return 1;
    }

    auto const host = argv[1];
    auto const port = argv[2];

    net::io_context ioc;
    std::string username = "root";
    std::string password = "****";
    std::make_shared<session>(ioc, username, password)->run(host, port);
    ioc.run();

    return 0;
}
