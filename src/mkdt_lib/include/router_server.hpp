/*! @file router_server.hpp
 *
 */

#ifndef MKDT_ROUTER_SERVER_HPP
#define MKDT_ROUTER_SERVER_HPP

#include <boost/asio.hpp>

namespace mkdt {

class router_server {
public:
    router_server() = default;

    router_server(boost::asio::io_context &io_context);

    router_server(const router_server &) = delete;

    router_server &operator=(const router_server &) = delete;

};

}

#endif //MKDT_ROUTER_SERVER_HPP
