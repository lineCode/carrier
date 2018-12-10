#include <file_transfer_client_async.hpp>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port> [<file|dir>] ..." << std::endl;
        return 1;
    }

    net::io_context ioc;
    auto session_ = std::make_shared<session>(ioc, argv[1], argv[2]);

    std::thread t([&ioc]{ ioc.run(); });

    for (int i = 3; i != argc; ++i)
         session_->transfer(std::string(argv[i]));

    /*
    session_->transfer(fs::path("path/transfer/"));
    session_->transfer(std::string("[]<typename ...>(){}();\n"), "file/transfer.txt");
    */

    t.join();

    return 0;
}
