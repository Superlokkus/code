/*! @file router_server.hpp
 *
 */

#ifndef MKDT_ROUTER_SERVER_HPP
#define MKDT_ROUTER_SERVER_HPP

#include <boost/asio.hpp>

namespace mkdt {

class router_server final {
public:
    router_server() = delete;

    router_server(boost::asio::io_context &io_context);

    ~router_server();

    router_server(const router_server &) = delete;
    router_server &operator=(const router_server &) = delete;

private:
    boost::asio::ip::tcp::acceptor tcp_v4_;
    boost::asio::ip::tcp::acceptor tcp_v6_;
};

}

#endif //MKDT_ROUTER_SERVER_HPP
