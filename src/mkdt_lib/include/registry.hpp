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

#include <router.hpp>
#include <common_definitions.hpp>

namespace mkdt {


class registry final {
public:
    registry(boost::asio::io_context &io_context) :
            io_context_(io_context),
            registry_strand_(io_context_),
            router_(io_context_) {

    }

    struct object {
        virtual ~object() = default;
    };

    struct receiver : object {
        ~receiver() override = default;

        virtual void receive(const std::string &message, const object_identifier &sender) = 0;
    };

    struct object_factory : object {
        ~object_factory() override = default;

        virtual std::shared_ptr<receiver> create_service_endpoint_object(service_identifier service_id) = 0;
    };

    /*!
     * @param service_id
     */
    void register_stateless_service(service_identifier service_id,
                                    std::shared_ptr<receiver> service_object);

    /*!
     * @param service_id
     */
    void register_statefull_service(service_identifier service_id,
                                    std::shared_ptr<object_factory> factory);

    /*!
     *
     * @param service_id
     * @param handler
     */
    void use_service_interface(service_identifier service_id, std::function<void(object_identifier)> handler);

    /*!
     * @tparam IdentifierHandler h of Handler: h(object_identifier)
     * @param service_id
     * @param object
     * @param handler
     */
    template<typename EndpointObject, typename IdentifierHandler>
    void expose(service_identifier service_id, std::shared_ptr<receiver> object, IdentifierHandler &&handler);

    /*!
     *
     * @tparam ConsumeHandler h of ConsumeHandler: h(bool consume_success, object_identifier)
     * @param object
     * @param handler
     */
    template<typename ConsumeHandler>
    void consume(object_identifier object, ConsumeHandler &&handler);


    /*!
     * @param receiver
     * @param message
     * @param handler
     */
    void send_message_to_object(const object_identifier &receiver, const std::string &message,
                                std::function<void(void)> handler);

    boost::asio::io_context &get_executor() {
        return this->io_context_;
    }

private:
    boost::asio::io_context &io_context_;
    boost::asio::io_context::strand registry_strand_;
    mkdt::router_client router_;

    using service_object = boost::variant<std::shared_ptr<receiver>, std::shared_ptr<object_factory>>;
    std::unordered_map<object_identifier, service_object> services_;
};

}


#endif //MKDT_REGISTRY_HPP
