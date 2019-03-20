/*! @file registry.cpp
 *
 */

#include <registry.hpp>

void mkdt::registry::register_service(mkdt::service_identifier service_id,
                                      std::shared_ptr<mkdt::receiver> service_object,
                                      std::function<void(error)> completion_handler) {
    boost::asio::dispatch(this->io_context_, boost::asio::bind_executor(this->registry_strand_,
            [=, completion_handler = std::move(completion_handler)]() {
                const auto new_object_id = std::make_pair(service_id, this->uuid_gen_());
                this->services_.emplace(service_id,new_object_id);
                this->objects_.emplace(new_object_id, service_object);
                this->router_.register_service(service_id, std::move(completion_handler),
                        [=] (auto callback) {
                   boost::asio::dispatch(this->io_context_, boost::asio::bind_executor(this->registry_strand_,
                           [=]() {
                       callback(new_object_id);
                   }));
                });
    }));
}

void mkdt::registry::send_message_to_object(const mkdt::object_identifier &receiver, const std::string &message,
                                            std::function<void(error)> handler) {

}

void mkdt::registry::use_service_interface(mkdt::service_identifier service_id,
                                           std::function<void(error, object_identifier)> handler) {
    this->router_.use_service_interface(service_id, std::move(handler));
}


