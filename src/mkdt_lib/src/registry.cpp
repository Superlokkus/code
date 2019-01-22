/*! @file registry.cpp
 *
 */

#include <registry.hpp>

void mkdt::registry::register_service(mkdt::service_identifier service_id,
                                      std::shared_ptr<mkdt::registry::receiver> service_object,
                                      std::function<void(void)> completion_handler) {
    this->router_.register_service(service_id, std::move(completion_handler));
    boost::asio::dispatch(this->io_context_, boost::asio::bind_executor(this->registry_strand_,
                                                                        [=]() {
                                                                        this->services_.emplace(service_id,
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

