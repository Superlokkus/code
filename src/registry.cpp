/*! @file registry.cpp
 *
 */

#include <registry.hpp>


std::future<void> mkdt::object_socket::send(std::string message) {
    return std::future<void>();
}

std::future<void> mkdt::object_socket::recieve(mkdt::incoming_callback incoming_message_callback) {
    return std::future<void>();
}

std::future<void>
mkdt::registry::register_stateless_service(mkdt::service_identifier service_id, mkdt::incoming_callback) {
    return std::future<void>();
}

std::future<void> mkdt::registry::register_statefull_service(mkdt::service_identifier service_id,
                                                             mkdt::service_factory_callback factory_callback) {
    return std::future<void>();
}

std::future<mkdt::object_handle> mkdt::registry::use_service_interface(mkdt::service_identifier service_id) {
    return std::future<mkdt::object_handle>();
}

std::future<mkdt::object_identifier> mkdt::registry::expose(mkdt::service_identifier service_id) {
    return std::future<mkdt::object_identifier>();
}

std::future<mkdt::object_handle> mkdt::registry::consume(mkdt::object_identifier object) {
    return std::future<mkdt::object_handle>();
}
