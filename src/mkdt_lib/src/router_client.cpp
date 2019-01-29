/*! @file router_client.cpp
 *
 */

#include <router_client.hpp>

#include <mkdt_protocol.hpp>

mkdt::router_client::router_client(boost::asio::io_context &io_context) :
        router_io_context_(io_context),
        router_strand_(router_io_context_),
        local_socket_(router_io_context_),
        local_resolver_(router_io_context_) {
}

mkdt::router_client::router_client(boost::asio::io_context &io_context, uint16_t port) :
        router_client(io_context) {
    this->port_ = port;
}

void mkdt::router_client::register_service(mkdt::service_identifier service_id,
                                           std::function<void(void)> handler,
                                           std::function<void(std::function<void(object_identifier)>)>
                                           new_service_request_handler) {
    boost::asio::dispatch(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::register_new_service, this, service_id, std::move(handler)
    )));
}

void mkdt::router_client::register_new_service(mkdt::service_identifier service_id,
                                               std::function<void(void)> user_handler) {
    auto after_open = [=]() {
        boost::asio::post(this->router_io_context_, std::move(user_handler));
    };
    if (!this->local_socket_.is_open()) {
        this->open_socket(after_open);
    } else {
        after_open();
    }
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

void mkdt::router_client::inbound_local_message_handler(const boost::system::error_code &error,
                                                        std::size_t bytes_transferred) {

}

template<typename callback>
void mkdt::router_client::open_socket(callback &&then) {
    const std::string host {"localhost"};
    this->local_resolver_.async_resolve(host,std::to_string(this->port_),
            boost::asio::bind_executor(this->router_strand_,
            [this,then] (auto error, auto results) {
        if (error)
            throw std::runtime_error{error.message()};

        auto first_entry = results.cbegin();
        if (first_entry == results.cend()){
            throw std::runtime_error{std::string{"Could not resolve host"}};
        }

        this->local_socket_.async_connect(first_entry->endpoint(), boost::asio::bind_executor(this->router_strand_,
                [this,then] (auto error) {
            if (error)
                throw std::runtime_error{std::string{"Could not connect: "} + error.message()};

            boost::asio::async_read_until(this->local_socket_, this->in_streambuf_,
                            boost::asio::string_view{"mkdt_local_message_end\r\n"},
                                                  boost::asio::bind_executor(this->router_strand_,std::bind(
                                                          &mkdt::router_client::inbound_local_message_handler,
                                                          this, std::placeholders::_1, std::placeholders::_2
                                                          )));

            then();
        }));

    }));
}

