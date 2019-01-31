/*! @file router_client.cpp
 *
 */

#include <router_client.hpp>

mkdt::router_client::router_client(boost::asio::io_context &io_context) :
        router_io_context_(io_context),
        router_strand_(router_io_context_),
        local_resolver_(router_io_context_) {
}

mkdt::router_client::router_client(boost::asio::io_context &io_context, uint16_t port) :
        router_client(io_context) {
    this->port_ = port;
}

void mkdt::router_client::register_service(mkdt::service_identifier service_id,
                                           std::function<void(error)> handler,
                                           std::function<void(std::function<void(object_identifier)>)>
                                           new_service_request_handler) {
    boost::asio::dispatch(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, [=]() {
        mkdt::protocol::register_service_message register_message{"", service_id};
        this->send_request(register_message, [=](auto error, auto response) {
            if (error) {
                boost::asio::post(this->router_io_context_, std::bind(handler, error));
            } else {
                boost::asio::post(this->router_io_context_, std::bind(handler, protocol::response_to_error(response)));
            }

        });
    }));
}

void mkdt::router_client::send_request(const mkdt::protocol::local_request &request,
                                       std::function<void(error, mkdt::protocol::local_response)> handler) {
    auto request_text = std::make_shared<std::string>();
    boost::spirit::karma::generate(std::back_inserter(*request_text), mkdt::protocol::generate_local_request_grammar
            <std::back_insert_iterator<std::string>>{}, request);


    auto send = [this](std::shared_ptr<std::string> message_string, auto user_completion_handler) {
        this->request_completion_handlers_.push_back(user_completion_handler);
        this->last_usable_connection_->send(std::move(*message_string));
    };

    auto after_open = [=](auto error) {
        if (error) {
            boost::asio::post(this->router_io_context_, std::bind(handler, error, mkdt::protocol::local_response{}));
        } else {
            send(request_text, handler);
        }
    };
    if (!this->last_usable_connection_ || !this->last_usable_connection_->socket().is_open()) {
        this->open_socket(after_open);
    } else {
        after_open(mkdt::error{});
    }
}


void mkdt::router_client::use_service_interface(mkdt::service_identifier service_id,
                                                std::function<void(error, object_identifier)> handler) {
    boost::asio::post(this->router_io_context_, boost::asio::bind_executor(this->router_strand_, std::bind(
            &router_client::service_lookup, this, service_id, std::move(handler)
    )));
}

void mkdt::router_client::service_lookup(mkdt::service_identifier service_id,
                                         std::function<void(error, object_identifier)> user_handler) {

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

                this->last_usable_connection_ = std::make_shared<tcp_connection>(this->router_io_context_, *this);
                for (auto &handler : this->request_completion_handlers_) {
                    handler(mkdt::error{"TCP connection closed"}, protocol::simple_confirm{});
                }
                this->request_completion_handlers_.clear();
                last_usable_connection_->socket().async_connect(first_entry->endpoint(),
                                                                boost::asio::bind_executor(this->router_strand_,
                [this,then] (auto error) {
            if (error)
                return then(mkdt::error{error});
                    this->last_usable_connection_->start();

                    then(mkdt::error{});
        }));

    }));
}

mkdt::router_client::tcp_connection::tcp_connection(boost::asio::io_context &io_context,
                                                    mkdt::router_client &client)
        : io_context_(io_context),
          client_(client),
          socket_(io_context_),
          write_strand_(io_context_) {

}

boost::asio::ip::tcp::socket &mkdt::router_client::tcp_connection::socket() {
    return this->socket_;
}

void mkdt::router_client::tcp_connection::start() {
    boost::asio::async_read_until(this->socket_, this->in_streambuf_,
                                  boost::asio::string_view{"mkdt_local_message_end\r\n"},
                                  [me = shared_from_this()](
                                          const boost::system::error_code &error,
                                          std::size_t bytes_transferred) {
                                      return me->message_read(error, bytes_transferred);
                                  });
}

void mkdt::router_client::tcp_connection::message_read(const boost::system::error_code &error,
                                                       std::size_t bytes_transferred) {
    if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
        return;
    }
    if (error)
        throw std::runtime_error{error.message()};

    std::istream stream(&this->in_streambuf_);
    mkdt::protocol::local_message message;
    auto stream_it = std::istreambuf_iterator<char>(stream);
    this->parser_buffer_.clear();
    std::copy_n(stream_it, bytes_transferred, std::back_inserter(parser_buffer_));
    if (bytes_transferred > 0)
        ++stream_it;

    auto begin = parser_buffer_.cbegin(), end = parser_buffer_.cend();
    bool valid = boost::spirit::qi::parse(begin, end,
                                          mkdt::protocol::local_message_grammar<std::string::const_iterator>{},
                                          message);

    boost::optional<mkdt::protocol::local_response> response;
    if (!valid) {
        throw mkdt::error{"Server response unreadable"};
    } else {
        response = this->client_.process_message(message);
    }

    if (response) {
        std::string response_string;
        boost::spirit::karma::generate(std::back_inserter(response_string),
                                       mkdt::protocol::generate_local_response_grammar
                                               <std::back_insert_iterator<std::string>>{}, *response);

        this->send(std::move(response_string));
    }


    this->start();
}

void mkdt::router_client::tcp_connection::send(std::string message) {
    this->io_context_.dispatch(boost::asio::bind_executor(this->write_strand_,
                                                          [me = shared_from_this(), message = std::move(message)]() {
                                                              const auto write_in_progress = !me->send_queue_.empty();
                                                              me->send_queue_.push_back(std::move(message));
                                                              if (!write_in_progress)
                                                                  me->start_sending_queue();
                                                          }));
}

void mkdt::router_client::tcp_connection::start_sending_queue() {
    boost::asio::async_write(this->socket_, boost::asio::buffer(this->send_queue_.front()),
                             boost::asio::bind_executor(this->write_strand_, [me = shared_from_this()](auto error,
                                                                                                       auto bytes_transferred) {
                                 me->response_sent(error, bytes_transferred);
                             }));
}

void mkdt::router_client::tcp_connection::response_sent(const boost::system::error_code &error,
                                                        std::size_t bytes_transferred) {
    if (error)
        throw std::runtime_error{error.message()};
    this->send_queue_.pop_front();
    if (!this->send_queue_.empty())
        this->start_sending_queue();
}

boost::optional<mkdt::protocol::local_response>
mkdt::router_client::process_message(const mkdt::protocol::local_message &message) {
    if (message.which() == 1) {
        this->request_completion_handlers_.front()(mkdt::error{}, boost::get<protocol::local_response>(message));
        this->request_completion_handlers_.pop_front();
    }

    return boost::optional<mkdt::protocol::local_response>();
}

