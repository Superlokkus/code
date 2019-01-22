/*! @file router_client.cpp
 *
 */

#include <router_server.hpp>

#include <array>
#include <string>
#include <memory>

#include <mkdt_protocol.hpp>

struct tcp_connection : std::enable_shared_from_this<tcp_connection> {
    tcp_connection() = delete;

    tcp_connection(boost::asio::io_context &io_context)
            : socket_(io_context) {
    }

    boost::asio::ip::tcp::socket &socket() {
        return socket_;
    }

    void start() {
        boost::asio::async_read_until(this->socket_, this->in_streambuf_,
                                      boost::asio::string_view{"mkdt_local_message_end\r\n"},
                                      [me = shared_from_this()](
                                              const boost::system::error_code &error,
                                              std::size_t bytes_transferred) {
                                          return me->message_read(error, bytes_transferred);
                                      });
    }

    void message_read(const boost::system::error_code &error,
                      std::size_t bytes_transferred) {
        if (error)
            throw std::runtime_error{error.message()};


        this->start();
    }


    void response_sent(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        if (error)
            throw std::runtime_error{error.message()};

    }

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf in_streambuf_;
};

void start_async_receive(boost::asio::ip::tcp::acceptor &acceptor);

void handle_new_tcp_connection(const boost::system::error_code &error,
                               boost::asio::ip::tcp::acceptor &acceptor,
                               std::shared_ptr<tcp_connection> new_connection) {
    if (error)
        throw std::runtime_error(error.message());
    new_connection->start();
    start_async_receive(acceptor);
}

void start_async_receive(boost::asio::ip::tcp::acceptor &acceptor) {
    auto new_connection = std::make_shared<tcp_connection>(acceptor.get_io_context());
    acceptor.async_accept(new_connection->socket(),
                          std::bind(&handle_new_tcp_connection,
                                    std::placeholders::_1,
                                    std::ref(acceptor),
                                    new_connection)
    );
}

mkdt::router_server::router_server(boost::asio::io_context &io_context) :
        tcp_v4_{io_context, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(),
                                                           mkdt::mkdt_server_port_number}},
        tcp_v6_{io_context, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v6(),
                                                           mkdt::mkdt_server_port_number}} {

}

mkdt::router_server::~router_server() {

}
