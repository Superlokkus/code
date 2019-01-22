/*! @file router_client.cpp
 *
 */

#include <router_client.hpp>

#include <mkdt_protocol.hpp>

mkdt::router_client::router_client(boost::asio::io_context &io_context) :
        router_io_context_(io_context),
        router_strand_(router_io_context_) {
}


void mkdt::router_client::register_service(mkdt::service_identifier service_id,
                                           std::function<void(void)> handler) {
    boost::asio::dispatch(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::register_new_service_in_cache, this, service_id, std::move(handler)
    )));
}

void mkdt::router_client::register_new_service_in_cache(mkdt::service_identifier service_id,
                                                        std::function<void(void)> user_handler) {
    boost::asio::post(this->router_io_context_, std::move(user_handler));
}

void mkdt::router_client::use_service_interface(mkdt::service_identifier service_id,
                                                std::function<void(object_identifier)> handler) {
    boost::asio::post(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::service_lookup, this, service_id, std::move(handler)
    )));
}

void mkdt::router_client::service_lookup(mkdt::service_identifier service_id,
                                         std::function<void(object_identifier)> user_handler) {

}


