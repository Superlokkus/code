/*! @file router_client.cpp
 *
 */

#include <router_server.hpp>

#include <array>
#include <string>
#include <memory>
#include <functional>

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
        response = this->server_->process_request(request);
    }

    std::string response_string;
    boost::spirit::karma::generate(std::back_inserter(response_string), mkdt::protocol::generate_local_response_grammar
            <std::back_insert_iterator<std::string>>{}, response);

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
                                                           port}} {
}

void mkdt::router_server_spimpl::start() {
    this->start_async_receive(this->tcp_v4_);
    this->start_async_receive(this->tcp_v6_);
}

mkdt::protocol::local_response
mkdt::router_server_spimpl::process_request(const mkdt::protocol::local_request &request) {
    return mkdt::protocol::local_response();
}

mkdt::router_server_spimpl::~router_server_spimpl() = default;

mkdt::router_server::~router_server() = default;
