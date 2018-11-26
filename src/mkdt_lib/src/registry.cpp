/*! @file registry.cpp
 *
 */

#include <registry.hpp>

mkdt::registry::registry(boost::asio::io_context &io_context) :
        io_context_(io_context) {

}

void mkdt::registry::register_stateless_service(mkdt::service_identifier service_id,
                                                std::shared_ptr<mkdt::registry::receiver> service_object) {

}

void mkdt::registry::register_statefull_service(mkdt::service_identifier service_id,
                                                std::shared_ptr<mkdt::registry::object_factory> factory) {

}
