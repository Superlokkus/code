/*! @file common_definitions.hpp
 *
 */

#ifndef MKDT_COMMON_DEFINITIONS_HPP
#define MKDT_COMMON_DEFINITIONS_HPP

#include <string>
#include <boost/uuid/uuid.hpp>

namespace mkdt {

using service_identifier = std::string;

using object_identifier = boost::uuids::uuid;

const unsigned mkdt_server_port_number{1021u}; //!< Currently exp1, must be changed

}

namespace std {
template<>
struct hash<mkdt::object_identifier> {
    typedef mkdt::object_identifier argument_type;
    typedef std::size_t result_type;

    result_type operator()(argument_type const &object_identifier) const noexcept {
        return boost::uuids::hash_value(object_identifier);
    }
};
}

#endif //MKDT_COMMON_DEFINITIONS_HPP
