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

using incoming_callback = std::function<void (std::string)>;

class object_handle final {
public:
    std::future<void> send(std::string message);

    std::future<void> recieve(incoming_callback incoming_message_callback);

private:
    boost::uuids::uuid id;
    service_identifier service_id;
};


class registry final {
public:

    std::future<void> register_service(service_identifier service_id,
                                       incoming_callback incoming_message_callback);

    std::future<object_handle> use_service_interface(service_identifier service_id);

    std::future<object_identifier> expose(service_identifier service_id,
                                          incoming_callback incoming_message_callback);

    std::future<object_handle> consume(object_identifier object);



};


}

#endif //MKDT_REGISTRY_HPP
