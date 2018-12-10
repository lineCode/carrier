#ifndef WEBSOCKET_ASYNC_SSL_HPP
#define WEBSOCKET_ASYNC_SSL_HPP

#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <websocket.hpp>

namespace ssl = net::ssl;
using socket_t = websocket_t<ssl::stream<tcp::socket>>;

#endif
