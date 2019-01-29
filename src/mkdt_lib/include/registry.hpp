/*! @file registry.hpp
 *
 */

#ifndef MKDT_REGISTRY_HPP
#define MKDT_REGISTRY_HPP

#include <string>
#include <future>
#include <memory>
#include <functional>
#include <utility>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <router_client.hpp>
#include <common_definitions.hpp>

namespace mkdt {


class registry final {
public:
    registry(boost::asio::io_context &io_context) :
            io_context_(io_context),
            registry_strand_(io_context_),
            router_(io_context_) {

    }

    registry(boost::asio::io_context &io_context, uint16_t custom_port) :
            io_context_(io_context),
            registry_strand_(io_context_),
            router_(io_context_, custom_port) {

    }

    struct object {
        virtual ~object() = default;
    };

    struct receiver : object {
        ~receiver() override = default;

        virtual void receive(const std::string &message, boost::optional<object_identifier> sender) = 0;
    };


    /*!
     * @param service_id
     */
    void register_service(service_identifier service_id,
                          std::shared_ptr<receiver> service_object,
                          std::function<void(error)> completion_handler);

    /*!
     *
     * @param service_id
     * @param handler
     */
    void use_service_interface(service_identifier service_id, std::function<void(error, object_identifier)> handler);

    /*!
     * @tparam IdentifierHandler h of Handler: h(error,object_identifier)
     * @param service_id
     * @param object
     * @param handler
     */
    template<typename EndpointObject, typename IdentifierHandler>
    void expose(service_identifier service_id, std::shared_ptr<receiver> object, IdentifierHandler &&handler);

    /*!
    *
    * @param object
    * @param handler
    */
    void consume(object_identifier object, std::function<void(error, bool, object_identifier)> handler);


    /*!
     * @param receiver
     * @param message
     * @param handler
     */
    void send_message_to_object(const object_identifier &receiver, const std::string &message,
                                std::function<void(error)> handler);

    boost::asio::io_context &get_executor() {
        return this->io_context_;
    }

private:
    boost::asio::io_context &io_context_;
    boost::asio::io_context::strand registry_strand_;
    mkdt::router_client router_;

    boost::uuids::random_generator uuid_gen_{};

    std::unordered_map<service_identifier, object_identifier> services_;
    std::unordered_map<object_identifier, std::shared_ptr<receiver>> objects_;
};

}


#endif //MKDT_REGISTRY_HPP
