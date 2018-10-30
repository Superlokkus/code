/*! @file registry.hpp
 *
 */

#ifndef MKDT_REGISTRY_HPP
#define MKDT_REGISTRY_HPP

#include <string>
#include <future>
#include <memory>

#include <boost/uuid/uuid.hpp>

namespace mkdt {

using service_identifier = std::string;

using object_identifier = boost::uuids::uuid;


class registry {
public:

    template<typename Service>
    std::future<void> register_service(service_identifier service_id, std::shared_ptr<Service> service);

    template<typename ServiceInterfaceStub>
    std::future<ServiceInterfaceStub> get_service_interface(service_identifier service_id);

    template<typename Object>
    std::future<object_identifier> expose(std::shared_ptr<Object> object, service_identifier service_id);

    template<typename Object>
    std::future<Object> consume(object_identifier object);

    std::future<void> send_message(object_identifier to_object, std::shared_ptr<std::string> message);

    void receive_message(std::function<void(std::string)> callback);



};


}

#endif //MKDT_REGISTRY_HPP
