#include <websocket_server_async_ssl.hpp>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage:    " << argv[0] << " <host> <port>\n"
                  << "Example:  " << argv[0] << " 0.0.0.0 80\n";
        return 1;
    }

    auto const host = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = static_cast<int>(std::thread::hardware_concurrency());

    net::io_context ioc{threads};
    std::make_shared<listener>(ioc, endpoint_t{host, port})->run();

    std::vector<std::thread> v;
    v.reserve(threads - 1);

    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc]{ ioc.run(); });
    ioc.run();

    return 0;
}
