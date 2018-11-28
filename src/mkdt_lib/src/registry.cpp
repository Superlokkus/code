/*! @file registry.cpp
 *
 */

#include <registry.hpp>

void mkdt::registry::register_stateless_service(mkdt::service_identifier service_id,
                                                std::shared_ptr<mkdt::registry::receiver> service_object) {
    this->router_.register_service(service_id, boost::asio::bind_executor(this->registry_strand_,
                                                                          [this, service_object](auto object_id) {
                                                                              this->services_.emplace(object_id,
                                                                                                      service_object);
                                                                          }));
}

void mkdt::registry::send_message_to_object(const mkdt::object_identifier &receiver, const std::string &message,
                                            std::function<void(void)> handler) {

}

void mkdt::registry::use_service_interface(mkdt::service_identifier service_id,
                                           std::function<void(object_identifier)> handler) {
    this->router_.use_service_interface(service_id, std::move(handler));
}

