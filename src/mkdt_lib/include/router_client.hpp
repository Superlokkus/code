/*! @file router_client.hpp
 *
 */

#ifndef MKDT_INTERFACE_ROUTER_HPP
#define MKDT_INTERFACE_ROUTER_HPP

#include <unordered_map>
#include <functional>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <common_definitions.hpp>
#include <mkdt_protocol.hpp>

namespace mkdt {

class router_client {
public:
    router_client() = delete;

    router_client(boost::asio::io_context &io_context);

    router_client(boost::asio::io_context &io_context, uint16_t port);

    /*!
     * @param service_id
     *
     */
    void register_service(service_identifier service_id, std::function<void(error)> handler,
                          std::function<void(std::function<void(object_identifier)>)> new_service_request_handler);

    void unregister_service(object_identifier object_id);
    /*!
     *
     * @param service_id
     * @param handler
     */
    void use_service_interface(service_identifier service_id, std::function<void(error, object_identifier)> handler);

    /*!
     * @tparam EndpointObject Must fullfill for values s of EndpointObject: s->receive(const std::string& message,
     * const object_identifier& sender)
     * @tparam IdentifierHandler h of Handler: h(error,object_identifier)
     * @param service_id
     * @param object
     * @param handler
     */
    template<typename EndpointObject, typename IdentifierHandler>
    void expose(service_identifier service_id, std::shared_ptr<EndpointObject> object, IdentifierHandler &&handler);

    /*!
     *
     * @param object
     * @param handler
     */
    void consume(object_identifier object, std::function<void(error, bool, object_identifier)> handler);


    /*!
     *
     * @tparam CompletionHandler
     * @param receiver
     * @param message
     * @param handler
     */
    template<typename CompletionHandler>
    void send_message_to_object(const object_identifier &receiver, const std::string &message,
                                CompletionHandler &&handler);

private:
    struct tcp_connection;
    boost::asio::io_context &router_io_context_;
    boost::asio::io_context::strand router_strand_;
    boost::asio::ip::tcp::resolver local_resolver_;
    uint16_t port_{mkdt::mkdt_server_port_number};
    std::shared_ptr<tcp_connection> last_usable_connection_;

    boost::uuids::random_generator uuid_gen_;
    std::unordered_map<object_identifier, service_identifier> object_id_to_service_id_;

    void service_lookup(service_identifier service_id,
                        std::function<void(error, object_identifier)> user_handler);

    void send_request(const mkdt::protocol::local_request &request, std::function<void(error,
                                                                                       mkdt::protocol::local_response)>);

    /*! For use in already synchronized methods, unsychronized
     * @param then Will be called after socket has been connected, sychronized with strand
     */
    template<typename callback>
    void open_socket(callback &&then);

    boost::optional<mkdt::protocol::local_response> process_message(const mkdt::protocol::local_message &message);

    struct tcp_connection : std::enable_shared_from_this<tcp_connection> {
        tcp_connection() = delete;

        tcp_connection(boost::asio::io_context &io_context, router_client &client);

        boost::asio::ip::tcp::socket &socket();

        void start();

        void send(std::string message);

    private:
        boost::asio::io_context &io_context_;
        router_client &client_;
        boost::asio::ip::tcp::socket socket_;
        boost::asio::streambuf in_streambuf_;
        std::string parser_buffer_;

        boost::asio::io_context::strand write_strand_;
        std::deque<std::string> send_queue_;

        void message_read(const boost::system::error_code &error,
                          std::size_t bytes_transferred);

        void response_sent(const boost::system::error_code &error,
                           std::size_t bytes_transferred);

        void start_sending_queue();

    };
};
}

#endif //MKDT_INTERFACE_ROUTER_HPP
