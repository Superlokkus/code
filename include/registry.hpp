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

using object_handle = boost::uuids::uuid;


class registry {
public:

    std::future<void> register_service(service_identifier service_id,
                                       incoming_callback incoming_message_callback);

    std::future<void> use_service_interface(service_identifier service_id, std::string message);

    std::future<object_identifier> expose(service_identifier service_id,
                                          incoming_callback incoming_message_callback);

    std::future<object_handle> consume(object_identifier object);

    std::future<void> send_message_to_object(object_handle object, std::string message);




};


}

#endif //MKDT_REGISTRY_HPP
