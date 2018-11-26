/*! @file registry.hpp
 *
 */

#ifndef MKDT_REGISTRY_HPP
#define MKDT_REGISTRY_HPP

#include <string>
#include <future>
#include <memory>
#include <functional>

#include <boost/uuid/uuid.hpp>
#include <boost/asio.hpp>

namespace mkdt {

using service_identifier = std::string;

using object_identifier = boost::uuids::uuid;


class registry final {
public:
    registry(boost::asio::io_context &io_context);

    /*!
     * @param service_id
     * @tparam ServiceEndpointObject Must fullfill for values s of ServiceEndpointObject: s->receive(const std::string& message,
     * const object_identifier& sender)
     */
    template<typename ServiceEndpointObject>
    void register_stateless_service(service_identifier service_id,
                                    std::shared_ptr<ServiceEndpointObject> service_object);

    /*!
     * @param service_id
     * @tparam ServiceEndpointFactory Must fullfill for values f of ServiceEndpointFactory:
     * std::shared_ptr<ServiceEndpointObject> f->create_service_endpoint_object(service_identifier service_id),
     * s of ServiceEndpointObject: s->receive(const std::string& message, const object_identifier& sender)
     */
    template<typename ServiceEndpointFactory>
    void register_statefull_service(service_identifier service_id,
                                    std::shared_ptr<ServiceEndpointFactory> factory);

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
    boost::asio::io_context &io_context_;
};

}


mkdt::registry::registry(boost::asio::io_context &io_context) :
        io_context_(io_context) {

}

template<typename ServiceEndpointObject>
void mkdt::registry::register_stateless_service(mkdt::service_identifier service_id,
                                                std::shared_ptr<ServiceEndpointObject> service_object) {

}

template<typename ServiceEndpointFactory>
void mkdt::registry::register_statefull_service(mkdt::service_identifier service_id,
                                                std::shared_ptr<ServiceEndpointFactory> factory) {

}

template<typename IdentifierHandler>
void mkdt::registry::use_service_interface(mkdt::service_identifier service_id, IdentifierHandler &&handler) {

}

template<typename EndpointObject, typename IdentifierHandler>
void mkdt::registry::expose(mkdt::service_identifier service_id, std::shared_ptr<EndpointObject> object,
                            IdentifierHandler &&handler) {

}

template<typename ConsumeHandler>
void mkdt::registry::consume(mkdt::object_identifier object, ConsumeHandler &&handler) {

}

template<typename CompletionHandler>
void mkdt::registry::send_message_to_object(const mkdt::object_identifier &receiver, const std::string &message,
                                            CompletionHandler &&handler) {

}

#endif //MKDT_REGISTRY_HPP
