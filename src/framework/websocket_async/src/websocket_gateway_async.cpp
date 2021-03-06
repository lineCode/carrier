#include <websocket_gateway_async.hpp>
    
int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port> <conf>\n";
        return 1;
    }

    auto const host = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = static_cast<int>(std::thread::hardware_concurrency());
    
    net::io_context ioc{threads};
    auto services = loadconfig(argv[3]);
    std::make_shared<listener>(ioc, endpoint_t{host, port})->run(services);

    std::vector<std::thread> v;
    v.reserve(threads - 1);

    for (auto i = threads - 1; i > 0; --i)
         v.emplace_back([&ioc]{ ioc.run(); });
    ioc.run();

    return 0;
}
