/*! @file router.hpp
 *
 */

#ifndef MKDT_INTERFACE_ROUTER_HPP
#define MKDT_INTERFACE_ROUTER_HPP

#include <unordered_map>
#include <functional>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <common_definitions.hpp>

namespace mkdt {

class router_client {
public:
    router_client() = delete;

    router_client(boost::asio::io_context &io_context);

    /*!
     * @param service_id
     */
    void register_service(service_identifier service_id, std::function<void(object_identifier)> handler);

    void unregister_service(object_identifier object_id);
    /*!
     *
     * @tparam IdentifierHandler h of Handler: h(object_identifier)
     * @param service_id
     * @param handler
     */
    template<typename IdentifierHandler>
    void use_service_interface(service_identifier service_id, IdentifierHandler &&handler);

    /*!
     * @tparam EndpointObject Must fullfill for values s of EndpointObject: s->receive(const std::string& message,
     * const object_identifier& sender)
     * @tparam IdentifierHandler h of Handler: h(object_identifier)
     * @param service_id
     * @param object
     * @param handler
     */
    template<typename EndpointObject, typename IdentifierHandler>
    void expose(service_identifier service_id, std::shared_ptr<EndpointObject> object, IdentifierHandler &&handler);

    /*!
     *
     * @tparam ConsumeHandler h of ConsumeHandler: h(bool consume_success, object_identifier)
     * @param object
     * @param handler
     */
    template<typename ConsumeHandler>
    void consume(object_identifier object, ConsumeHandler &&handler);


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
    boost::asio::io_context &router_io_context_;
    boost::asio::io_context::strand router_strand_;

    boost::uuids::random_generator uuid_gen_;
    std::unordered_map<object_identifier, service_identifier> object_id_to_service_id_;

    void
    register_new_service_in_cache(service_identifier service_id, std::function<void(object_identifier)> user_handler);
};

class router_server {

};

}

mkdt::router_client::router_client(boost::asio::io_context &io_context) :
        router_io_context_(io_context),
        router_strand_(router_io_context_) {
}

template<typename IdentifierHandler>
void mkdt::router_client::use_service_interface(mkdt::service_identifier service_id, IdentifierHandler &&handler) {

}

void mkdt::router_client::register_service(mkdt::service_identifier service_id,
                                           std::function<void(object_identifier)> handler) {
    boost::asio::post(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::register_new_service_in_cache, this, service_id, handler
    )));
}

void mkdt::router_client::register_new_service_in_cache(mkdt::service_identifier service_id,
                                                        std::function<void(object_identifier)> user_handler) {
    auto uuid = this->uuid_gen_();
    this->object_id_to_service_id_.emplace(uuid, service_id);
    boost::asio::post(this->router_io_context_, std::bind(user_handler, uuid));
}

#endif //MKDT_INTERFACE_ROUTER_HPP
