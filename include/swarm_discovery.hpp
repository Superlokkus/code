/*! @file swarm_discovery.hpp
 *
 */

#ifndef MKDT_INTERFACE_SWARM_DISCOVERY_HPP
#define MKDT_INTERFACE_SWARM_DISCOVERY_HPP

#include <stdexcept>
#include <string>

namespace mkdt {
namespace swarm_discovery {

const char *const comprising_service_name_ptr = "mkdt";
const std::string comprising_service_name{comprising_service_name_ptr};

struct swarm_discovery_exception : std::runtime_error {
    swarm_discovery_exception(int avahi_errno);
};

class swarm_discovery_service final {
public:
    swarm_discovery_service();

    ~swarm_discovery_service();

    void publish_for_service(std::string service_identifier);


};


}
}


#endif //MKDT_INTERFACE_SWARM_DISCOVERY_HPP
