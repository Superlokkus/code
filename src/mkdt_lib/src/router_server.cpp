/*! @file router_client.cpp
 *
 */

#include <router_server.hpp>

#include <array>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <iterator>

#include <boost/log/trivial.hpp>

void mkdt::router_server_spimpl::start_async_receive(boost::asio::ip::tcp::acceptor &acceptor) {
    auto new_connection = std::make_shared<tcp_connection>(acceptor.get_io_context(), shared_from_this());
    acceptor.async_accept(new_connection->socket(), std::bind(
            [me = shared_from_this()](const boost::system::error_code &error,
                                      boost::asio::ip::tcp::acceptor &acceptor,
                                      std::shared_ptr<mkdt::router_server_spimpl::tcp_connection> new_connection) {
                me->handle_new_tcp_connection(error, acceptor, new_connection);
            }, std::placeholders::_1, std::ref(acceptor), new_connection));
}

mkdt::router_server_spimpl::tcp_connection::tcp_connection(boost::asio::io_context &io_context,
                                                           std::shared_ptr<mkdt::router_server_spimpl> server)
        : io_context_(io_context),
          server_(std::move(server)),
          socket_(io_context_),
          write_strand_(io_context_) {

}

boost::asio::ip::tcp::socket &mkdt::router_server_spimpl::tcp_connection::socket() {
    return this->socket_;
}

void mkdt::router_server_spimpl::tcp_connection::start() {
    boost::asio::async_read_until(this->socket_, this->in_streambuf_,
                                  boost::asio::string_view{"mkdt_local_message_end\r\n"},
                                  [me = shared_from_this()](
                                          const boost::system::error_code &error,
                                          std::size_t bytes_transferred) {
                                      return me->message_read(error, bytes_transferred);
                                  });
}

void mkdt::router_server_spimpl::tcp_connection::message_read(const boost::system::error_code &error,
                                                              std::size_t bytes_transferred) {
    if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
        return;
    }
    if (error)
        throw std::runtime_error{error.message()};

    std::istream stream(&this->in_streambuf_);
    mkdt::protocol::local_request request;
    auto stream_it = std::istreambuf_iterator<char>(stream);
    this->parser_buffer_.clear();
    std::copy_n(stream_it, bytes_transferred, std::back_inserter(parser_buffer_));
    if (bytes_transferred > 0)
        ++stream_it;

    auto begin = parser_buffer_.cbegin(), end = parser_buffer_.cend();
    bool valid = boost::spirit::qi::parse(begin, end,
                                          mkdt::protocol::local_request_grammar<std::string::const_iterator>{},
                                          request);

    mkdt::protocol::local_response response;
    if (!valid) {
        response = mkdt::protocol::simple_confirm{400, "Bad Request"};
    } else {
        response = this->server_->process_request(request, shared_from_this());
    }

    BOOST_LOG_TRIVIAL(info) << parser_buffer_;

    std::string response_string;
    boost::spirit::karma::generate(std::back_inserter(response_string), mkdt::protocol::generate_local_response_grammar
            <std::back_insert_iterator<std::string>>{}, response);

    BOOST_LOG_TRIVIAL(info) << response_string;

    this->send(std::move(response_string));


    this->start();
}

void mkdt::router_server_spimpl::tcp_connection::send(std::string message) {
    this->io_context_.dispatch(boost::asio::bind_executor(this->write_strand_,
                                                          [me = shared_from_this(), message = std::move(message)]() {
                                                              const auto write_in_progress = !me->send_queue_.empty();
                                                              me->send_queue_.push_back(std::move(message));
                                                              if (!write_in_progress)
                                                                  me->start_sending_queue();
                                                          }));
}

void mkdt::router_server_spimpl::tcp_connection::start_sending_queue() {
    boost::asio::async_write(this->socket_, boost::asio::buffer(this->send_queue_.front()),
                             boost::asio::bind_executor(this->write_strand_, [me = shared_from_this()](auto error,
                                                                                                       auto bytes_transferred) {
                                 me->response_sent(error, bytes_transferred);
                             }));
}

void mkdt::router_server_spimpl::tcp_connection::response_sent(const boost::system::error_code &error,
                                                               std::size_t bytes_transferred) {
    if (error)
        throw std::runtime_error{error.message()};
    this->send_queue_.pop_front();
    if (!this->send_queue_.empty())
        this->start_sending_queue();
}

void mkdt::router_server_spimpl::handle_new_tcp_connection(const boost::system::error_code &error,
                                                           boost::asio::ip::tcp::acceptor &acceptor,
                                                           std::shared_ptr<mkdt::router_server_spimpl::tcp_connection> new_connection) {
    if (error)
        throw std::runtime_error(error.message());
    new_connection->start();
    start_async_receive(acceptor);
}


mkdt::router_server::router_server(boost::asio::io_context &io_context) :
        router_server_(std::make_shared<router_server_spimpl>(io_context)) {
    router_server_->start();
}

mkdt::router_server::router_server(boost::asio::io_context &io_context, uint16_t port) :
        router_server_(std::make_shared<router_server_spimpl>(io_context, port)) {
    router_server_->start();
}

mkdt::router_server_spimpl::router_server_spimpl(boost::asio::io_context &io_context) :
        router_server_spimpl(io_context, mkdt::mkdt_server_port_number) {
}

mkdt::router_server_spimpl::router_server_spimpl(boost::asio::io_context &io_context, uint16_t port) :
        io_context_(io_context),
        tcp_v4_{io_context, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(),
                                                           port}},
        tcp_v6_{io_context, boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v6(),
                                                           port}},
        default_random_engine_(std::random_device()()) {
}

void mkdt::router_server_spimpl::start() {
    this->start_async_receive(this->tcp_v4_);
    this->start_async_receive(this->tcp_v6_);
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::process_request(const mkdt::protocol::local_request &request,
                                            router_client_endpoint endpoint_making_request) {
    handle_request_visitor request_visitor{*this, endpoint_making_request};
    auto response = boost::apply_visitor(request_visitor, request);

    return response;
}

mkdt::router_server_spimpl::~router_server_spimpl() = default;

mkdt::router_server::~router_server() = default;

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const protocol::register_service_message &request) {
    protocol::simple_confirm response{200, "OK"};

    if (request.service_name.empty() || request.service_name == " ") {
        response.code = 422;
        response.text = "Will not register an empty or blank service name";
        return response;
    }

    const auto endpoint_range = this->server_.services_by_endpoint_.equal_range(this->current_endpoint_);
    std::pair<const router_client_endpoint, std::string> searched_for_value =
            std::make_pair(this->current_endpoint_, request.service_name);
    const auto entry = std::find(endpoint_range.first, endpoint_range.second,
                                 searched_for_value);
    if (entry != endpoint_range.second) {
        return response;
    } else {
        this->server_.services_by_endpoint_.insert(endpoint_range.first,
                                                   std::make_pair(this->current_endpoint_, request.service_name));
        this->server_.services_.emplace(request.service_name, this->current_endpoint_);
    }

    return response;
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const mkdt::protocol::unregister_service_message &) {
    protocol::simple_confirm response{200, "OK"};

    return response;
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const mkdt::protocol::use_service_request &request) {
    protocol::object_answer response{200, "OK"};

    if (request.service_name.empty() || request.service_name == " ") {
        response.request_in_general.code = 422;
        response.request_in_general.text = "Does not support empty or blank service names";
        return response;
    }

    auto service_endpoints_range = this->server_.services_.equal_range(request.service_name);
    if (service_endpoints_range.first == service_endpoints_range.second) {
        response.request_in_general.code = 404;
        response.request_in_general.text = std::string{"Service \""} + request.service_name + "\" not found";
        return response;
    }
    std::uniform_int_distribution<size_t> service_choosen_number_rd{0, static_cast<size_t>(//std::distance >= 0
                                                                               std::distance(
                                                                                       service_endpoints_range.first,
                                                                                       service_endpoints_range.second)) -
                                                                       1};
    std::advance(service_endpoints_range.first, service_choosen_number_rd(
            this->server_.default_random_engine_
    ));
    auto choosen_service_endpoint = service_endpoints_range.first->second;

    //TODO response with object id

    return response;
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const mkdt::protocol::expose_object_message &) {
    return mkdt::protocol::local_response();
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const mkdt::protocol::consume_object_request &) {
    return mkdt::protocol::local_response();
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::handle_request_visitor::operator()(const mkdt::protocol::message_for_object &) {
    return mkdt::protocol::local_response();
}
