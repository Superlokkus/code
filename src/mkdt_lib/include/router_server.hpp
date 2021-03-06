/*! @file router_server.hpp
 *
 */

#ifndef MKDT_ROUTER_SERVER_HPP
#define MKDT_ROUTER_SERVER_HPP

#include <cstdint>
#include <memory>
#include <deque>
#include <unordered_map>
#include <random>

#include <boost/asio.hpp>

#include <mkdt_protocol.hpp>

namespace mkdt {

class router_server_spimpl final : public std::enable_shared_from_this<router_server_spimpl> {
public:
    router_server_spimpl(boost::asio::io_context &io_context);

    router_server_spimpl(boost::asio::io_context &io_context, uint16_t port);

    void start();

    ~router_server_spimpl();

private:
    boost::asio::io_context &io_context_;
    boost::asio::ip::tcp::acceptor tcp_v4_;
    boost::asio::ip::tcp::acceptor tcp_v6_;
    std::default_random_engine default_random_engine_;

    struct tcp_connection;
    using router_client_endpoint = std::shared_ptr<tcp_connection>;

    mkdt::protocol::local_response process_request(const mkdt::protocol::local_request &request,
                                                   router_client_endpoint endpoint_making_request);

    using service_entry = router_client_endpoint;
    std::unordered_multimap<std::string, service_entry> services_;
    std::unordered_multimap<router_client_endpoint, std::string> services_by_endpoint_;

    struct tcp_connection : std::enable_shared_from_this<tcp_connection> {
        tcp_connection() = delete;

        tcp_connection(boost::asio::io_context &io_context, std::shared_ptr<router_server_spimpl> server);

        boost::asio::ip::tcp::socket &socket();

        void start();

    private:
        boost::asio::io_context &io_context_;
        std::shared_ptr<router_server_spimpl> server_;
        boost::asio::ip::tcp::socket socket_;
        boost::asio::streambuf in_streambuf_;
        std::string parser_buffer_;

        boost::asio::io_context::strand write_strand_;
        std::deque<std::string> send_queue_;

        void message_read(const boost::system::error_code &error,
                          std::size_t bytes_transferred);

        void response_sent(const boost::system::error_code &error,
                           std::size_t bytes_transferred);

        void send(std::string message);

        void start_sending_queue();

    };

    struct handle_request_visitor : boost::static_visitor<mkdt::protocol::local_response> {
        handle_request_visitor(router_server_spimpl &server, router_client_endpoint current_endpoint)
                : server_(server), current_endpoint_(current_endpoint) {}

        mkdt::protocol::local_response operator()(const protocol::register_service_message &);

        mkdt::protocol::local_response operator()(const protocol::unregister_service_message &);

        mkdt::protocol::local_response operator()(const protocol::use_service_request &);

        mkdt::protocol::local_response operator()(const protocol::expose_object_message &);

        mkdt::protocol::local_response operator()(const protocol::consume_object_request &);

        mkdt::protocol::local_response operator()(const protocol::message_for_object &);

    private:
        router_server_spimpl &server_;
        router_client_endpoint current_endpoint_;
    };

    void start_async_receive(boost::asio::ip::tcp::acceptor &acceptor);

    void handle_new_tcp_connection(const boost::system::error_code &error,
                                   boost::asio::ip::tcp::acceptor &acceptor,
                                   std::shared_ptr<tcp_connection> new_connection);
};

class router_server final {
public:
    router_server() = delete;

    /*! Uses the registered port, defined in mkdt::mkdt_server_port_number
     *
     * @param io_context
     */
    router_server(boost::asio::io_context &io_context);

    /*!
     *
     * @param io_context
     * @param port Custom port to use
     */
    router_server(boost::asio::io_context &io_context, uint16_t port);

    ~router_server();

    router_server(const router_server &) = delete;
    router_server &operator=(const router_server &) = delete;

    router_server &operator=(router_server &&) = default;

    router_server(router_server &&) = default;

private:
    std::shared_ptr<router_server_spimpl> router_server_;
};

}

#endif //MKDT_ROUTER_SERVER_HPP
