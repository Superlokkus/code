/*! @file router.hpp
 *
 */

#include <router.hpp>

mkdt::router_client::router_client(boost::asio::io_context &io_context) :
        router_io_context_(io_context),
        router_strand_(router_io_context_) {
}


void mkdt::router_client::register_service(mkdt::service_identifier service_id,
                                           std::function<void(object_identifier)> handler) {
    boost::asio::post(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::register_new_service_in_cache, this, service_id, handler
    )));
}

void mkdt::router_client::register_new_service_in_cache(mkdt::service_identifier service_id,
                                                        std::function<void(object_identifier)> user_handler) {
    auto uuid = this->uuid_gen_();
    this->object_id_to_service_id_.emplace(uuid, service_id);
    boost::asio::post(this->router_io_context_, std::bind(user_handler, uuid));
}

