#ifndef WEBSOCKET_SERVER_ASYNC_HPP
#define WEBSOCKET_SERVER_ASYNC_HPP

#include <websocket_async.hpp>
 
class session : public std::enable_shared_from_this<session>
{
    public:
        session(tcp::socket socket) :
        ws_(std::move(socket)), strand_(ws_.get_executor())
        {
            setup_stream(ws_);
        }

        std::shared_ptr<session> shared_this()
        {
            return shared_from_this();
        }

        void run()
        {
            ws_.async_accept(net::bind_executor(strand_, 
            [self = shared_this()](error_code_t ec)
            {
                self->on_accept(ec);
            }));
        }
    
        void on_accept(error_code_t ec)
        {
            if (ec)
                return fail(ec, "accept");

            do_read();
        }
    
        void do_read()
        {
            ws_.async_read(buffer_, net::bind_executor(strand_,
            [self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_read(ec, bytes_transferred);
            }));
        }

        void on_read(error_code_t ec, size_t bytes_transferred)
        {
            if (ec == websocket::error::closed || ec == net::error::eof)
                return;

            do_write();
        }

        void do_write()
        {
            ws_.async_write(buffer_.data(), net::bind_executor(strand_,
            [self = shared_this()](error_code_t ec, size_t bytes_transferred)
            {
                self->on_write(ec, bytes_transferred);
            }));
        }
    
        void on_write(error_code_t ec, size_t bytes_transferred)
        {
            if (ec)
                return fail(ec, "write");

            buffer_.consume(buffer_.size());
            do_read();
        }

    private:
        socket_t ws_;
        strand_t strand_;
        buffer_t buffer_;
};

class listener : public std::enable_shared_from_this<listener>
{
    public:
        listener(net::io_context& ioc, endpoint_t endpoint) :
        acceptor_(ioc), socket_(ioc)
        {
            error_code_t ec;
            acceptor_.open(endpoint.protocol(), ec);
            if (ec)
            {
                fail(ec, "open");
                return;
            }

            acceptor_.set_option(net::socket_base::reuse_address(true), ec);
            if (ec)
            {
                fail(ec, "set_option");
                return;
            }

            acceptor_.bind(endpoint, ec);
            if (ec)
            {
                fail(ec, "bind");
                return;
            }

            acceptor_.listen(net::socket_base::max_listen_connections, ec);
            if (ec)
            {
                fail(ec, "listen");
                return;
            }
        }

        void run()
        {
            if (! acceptor_.is_open())
                return;
            do_accept();
        }
    
        void do_accept()
        {
            acceptor_.async_accept(socket_,
            [self = shared_from_this()](error_code_t ec)
            {
                self->on_accept(ec);
            });
        }

        void on_accept(error_code_t ec)
        {
            if (ec)
                fail(ec, "accept");
            else
                std::make_shared<session>(std::move(socket_))->run();
            do_accept();
        }

    private:
        tcp::acceptor acceptor_;
        tcp::socket socket_;
};

#endif
