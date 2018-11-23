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
     *
     * @tparam IdentifierHandler h of Handler: h(object_identifier)
     * @param service_id
     * @param handler
     */
    template<typename IdentifierHandler>
    void expose(service_identifier service_id, IdentifierHandler &&handler);

    /*!
     *
     * @tparam ConsumeHandler h of ConsumeHandler: h(bool consume_success, object_identifier)
     * @param object
     * @param handler
     */
    template<typename ConsumeHandler>
    void consume(object_identifier object, ConsumeHandler &&handler);


    template<typename CompletionHandler>
    void send_message_to_object(const object_identifier &receiver, const std::string &message,
                                CompletionHandler &&handler);

private:
    boost::asio::io_context io_context_;
};


}

#endif //MKDT_REGISTRY_HPP
