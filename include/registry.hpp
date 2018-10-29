/*! @file registry.hpp
 *
 */

#ifndef MKDT_INTERFACE_HPP
#define MKDT_INTERFACE_HPP

#include <string>
#include <future>
#include <memory>

#include <boost/uuid/uuid.hpp>

namespace mkdt {

struct service_identifier : std::string {
    using std::string::string;
};

struct object_identifier : boost::uuids::uuid {
    using boost::uuids::uuid::uuid;
};

class registry {
public:

    template<typename Service>
    std::future<void> register_service(service_identifier component, std::shared_ptr<Service> service);

    template<typename ServiceInterfaceStub>
    std::future<ServiceInterfaceStub> get_service_interface(service_identifier service_id);

    template<typename Object>
    std::future<object_identifier> publish(std::shared_ptr<Object> object);

    template<typename Object>
    std::future<Object> subscribe(object_identifier object_id);

};


}

#endif
