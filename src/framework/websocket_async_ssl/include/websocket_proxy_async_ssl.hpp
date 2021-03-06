#ifndef WEBSOCKET_PROXY_ASYNC_SSL_HPP
#define WEBSOCKET_PROXY_ASYNC_SSL_HPP

#include <root_certificates.hpp>
#include <websocket_async_ssl.hpp>

using address_t = std::pair<std::string, std::string>;

class proxy : public std::enable_shared_from_this<proxy>
{
    public:
        explicit proxy(net::io_context& ioc) : ctx(ssl::context::sslv23_client),
        client(ioc, ctx), server(ioc, ctx), resolver(ioc)
        {
            setup_stream(client);
            setup_stream(server);
            load_root_certificates(ctx);
        }

        std::shared_ptr<proxy> shared_this()
        {
            return shared_from_this();
        }

        bool started()
        {
            return started_ == 2;
        }

        void start(const address_t& lhs, const address_t& rhs)
        {
             host_client = lhs.first;
             host_server = rhs.first;
             do_resolve(client, host_client, lhs.second);
             do_resolve(server, host_server, rhs.second);
        }

        void stop(socket_t& socket_)
        {
            if (started_ < 1)
                return;

            --started_;
            socket_.async_close(websocket::close_code::normal,
            [self = shared_this()](error_code_t ec)
            {
                self->on_close(ec);
            });
        }

        void on_close(error_code_t ec)
        {
            if (ec)
                return fail(ec, "close");
        }

        void do_resolve(socket_t& socket_, const std::string& host, const std::string& port)
        {
            resolver.async_resolve(host, port,
            [&, self = shared_this()](error_code_t ec, results_t results)
            {
                self->on_resolve(socket_, ec, results);
            });
        }

        void on_resolve(socket_t& socket_, error_code_t ec, results_t results)
        {
            if (ec)
                return fail(ec, "resolve");

            net::async_connect(socket_.next_layer().next_layer(), results.begin(), results.end(),
            std::bind(&proxy::on_connect, shared_this(), std::ref(socket_), std::placeholders::_1));
        }
    
        void on_connect(socket_t& socket_, error_code_t ec)
        {
            if (ec)
                return fail(ec, "connect");

            socket_.next_layer().async_handshake(ssl::stream_base::client,
            [&, self = shared_this()](error_code_t ec)
            {
                self->on_ssl_handshake(socket_, ec);
            });
        }
    
        void on_ssl_handshake(socket_t& socket_, error_code_t ec)
        {
            if (ec)
                return fail(ec, "ssl_handshake");

            auto& host = &socket_ == &client ? host_client : host_server;
            socket_.async_handshake(host, "/",
            [&, self = shared_this()](error_code_t ec)
            {
                self->on_handshake(socket_, ec);
            });
        }

        void on_handshake(socket_t& socket_, error_code_t ec)
        {
            if (ec)
            {
                stop(socket_);
                return fail(ec, "handshake");
            }

            if (++started_ == 2) 
            {
                do_read(client, buffer_client);
                do_read(server, buffer_server);
            }
        }

        void do_read(socket_t& socket_, buffer_t& buffer_)
        {
            socket_.async_read(buffer_,
            [&, self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_read(socket_, ec, bytes_transferred);
            });
        }

        void on_read(socket_t& socket_, error_code_t ec, size_t bytes_transferred)
        {
            if (ec == websocket::error::closed || ec == net::error::eof)
                return stop(socket_);

            if (! started())
                return;

            auto& buffer_ = &socket_ == &client ? buffer_client : buffer_server;
            do_write(&socket_ == &client ? server : client, buffer_, bytes_transferred);
        }

        void do_write(socket_t& socket_, buffer_t& buffer_, size_t bytes_transferred)
        {
            socket_.async_write(buffer_.data(),
            [&, self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_write(socket_, ec, bytes_transferred);
            });
        }

        void on_write(socket_t& socket_, error_code_t ec, size_t bytes_transferred)
        {
            if (ec)
            {
                stop(socket_);
                return fail(ec, "write");
            }

            auto& socket = &socket_ == &client ? server : client;
            auto& buffer = &socket_ == &client ? buffer_server : buffer_client;
            buffer.consume(buffer.size());
            do_read(socket, buffer);
        }

    private:
        int started_ = 0;
        ssl::context ctx;
        socket_t client;
        socket_t server;
        tcp::resolver resolver;
        buffer_t buffer_client;
        buffer_t buffer_server;
        std::string host_client;
        std::string host_server;
};

#endif
