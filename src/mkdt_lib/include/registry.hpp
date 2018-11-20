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

namespace mkdt {

using service_identifier = std::string;

using object_identifier = boost::uuids::uuid;

using incoming_callback = std::function<std::string(std::string)>;

using service_factory_callback = std::function<void(object_identifier)>;

class object_socket final {
    std::future<void> send(std::string message);
    std::future<void> recieve(incoming_callback incoming_message_callback);
};

class object_handle final {
public:
    object_socket socket;

private:
    boost::uuids::uuid id;
    service_identifier service_id;
};


class registry final {
public:

    std::future<void> register_stateless_service(service_identifier service_id,
                                                 incoming_callback);

    std::future<void> register_statefull_service(service_identifier service_id,
                                                 service_factory_callback factory_callback);

    std::future<object_handle> use_service_interface(service_identifier service_id);

    std::future<object_identifier> expose(service_identifier service_id);

    std::future<object_handle> consume(object_identifier object);


};


}

#endif //MKDT_REGISTRY_HPP
