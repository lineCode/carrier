#ifndef WEBSOCKET_CLIENT_ASYNC_SSL_HPP
#define WEBSOCKET_CLIENT_ASYNC_SSL_HPP

#include <root_certificates.hpp>
#include <websocket_async_ssl.hpp>

class session : public std::enable_shared_from_this<session>
{
    public:
        explicit session(net::io_context& ioc, ssl::context& ctx, int srv) :
        resolver_(ioc), ws_(ioc, ctx) , srv_(srv)
        {
            setup_stream(ws_);
        }

        std::shared_ptr<session> shared_this()
        {
            return shared_from_this();
        }

        void run(const std::string& host, const std::string& port, const std::string& text)
        {
            host_ = host;
            text_ = text;
            resolver_.async_resolve(host, port,
            [self = shared_this()](error_code_t ec, results_t results)
            {
                self->on_resolve(ec, results);
            });
        }

        void on_resolve(error_code_t ec, results_t results)
        {
            if (ec)
                return fail(ec, "resolve");

            net::async_connect(ws_.next_layer().next_layer(), results.begin(), results.end(),
            std::bind(&session::on_connect, shared_this(), std::placeholders::_1));
        }
    
        void on_connect(error_code_t ec)
        {
            if (ec)
                return fail(ec, "connect");

            ws_.next_layer().async_handshake(ssl::stream_base::client,
            [self = shared_this()](error_code_t ec)
            {
                self->on_ssl_handshake(ec);
            });
        }
    
        void on_ssl_handshake(error_code_t ec)
        {
            if (ec)
                return fail(ec, "ssl_handshake");

            ws_.async_handshake(host_, "/",
            [self = shared_this()](error_code_t ec)
            {
                self->on_handshake(ec);
            });
        }

        void on_handshake(error_code_t ec)
        {
            if (ec)
                return fail(ec, "handshake");

            do_write();
        }

        void do_write()
        {
            pb::carrier carrier_;
            carrier_.set_seq(10);
            carrier_.set_message(text_);
            carrier_.set_service(srv_);

            to_buffer(carrier_, buffer_);
            ws_.async_write(buffer_.data(),
            [self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_write(ec, bytes_transferred);
            });
        }

        void on_write(error_code_t ec, size_t bytes_transferred)
        {
            if (ec)
                return fail(ec, "write");

            ws_.async_read(buffer_,
            [self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_read(ec, bytes_transferred);
            });
        }

        void on_read(error_code_t ec, size_t bytes_transferred)
        {
            if (ec == websocket::error::closed || ec == net::error::eof)
                return;

            do_close();
        }

        void do_close()
        {
            ws_.async_close(websocket::close_code::normal,
            [self = shared_this()](error_code_t ec)
            {
                self->on_close(ec);
            });
        }

        void on_close(error_code_t ec)
        {
            if (ec)
                return fail(ec, "close");

            pb::carrier carrier_;
            if (! carrier_.ParseFromString(to_string(buffer_)))
                std::cout << "parse failed" << std::endl;
            std::cout << carrier_.message() << std::endl;
        }

    private:
        tcp::resolver resolver_;
        socket_t ws_;
        buffer_t buffer_;
        std::string host_;
        std::string text_;
        int srv_;

};

#endif
