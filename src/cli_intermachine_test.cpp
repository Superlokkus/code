/*! @file cli_intermachine_test.cpp
 *
 */

#include <iostream>
#include <array>
#include <string>
#include <memory>

#include <boost/asio.hpp>

struct tcp_connection : std::enable_shared_from_this<tcp_connection> {
    tcp_connection() = delete;

    /*! Constructs a new RTSP tcp connection
     *
     * @param io_context Boost asio io context to use for async calls on its socket
     * @param server_state RTSP server state for side effects of the tcp connection
     */
    tcp_connection(boost::asio::io_context &io_context)
            : socket_(io_context) {
    }

    boost::asio::ip::tcp::socket &socket() {
        return socket_;
    }

    /*! @brief Starts the session by reading in the rtsp header which should be blank line terminated
     *
     * Calls @ref header_read as handler for the read in raw handler
     */
    void start() {
        boost::asio::async_read_until(this->socket_, this->in_streambuf_, boost::asio::string_view{"end"},
                                      [me = shared_from_this()](
                                              const boost::system::error_code &error,
                                              std::size_t bytes_transferred) {
                                          return me->header_read(error, bytes_transferred);
                                      });
    }

    /*! @brief Parses the raw header and either sends a bad request response on failure or lets the
     * internal server state from construction handle the response and sends it's request
     * @param error Error from boost asio
     * @param bytes_transferred Bytes transfered from boost asio
     */
    void header_read(const boost::system::error_code &error,
                     std::size_t bytes_transferred) {
        if (error)
            throw std::runtime_error{error.message()};
        std::istream stream(&this->in_streambuf_);
        std::istreambuf_iterator<char> streami{stream};
        std::cout << "Input :" << std::string{streami, std::istreambuf_iterator<char>()} << "\n";
        this->start();
    }

    /*! @brief Handler stub to check for errors and upheld lifetime while sending async
     *
     * @param error Error from asio
     * @param bytes_transferred Bytes sent by asio
     */
    void response_sent(const boost::system::error_code &error,
                       std::size_t bytes_transferred) {
        if (error)
            throw std::runtime_error{error.message()};
        if (bytes_transferred != last_response_string_.size())
            throw std::runtime_error{"TCP bytes sent not equal"};

    }

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf in_streambuf_;
    std::string last_response_string_;
};

void start_async_receive(boost::asio::ip::tcp::acceptor &acceptor);

void handle_new_tcp_connection(const boost::system::error_code &error,
                               boost::asio::ip::tcp::acceptor &acceptor,
                               std::shared_ptr<tcp_connection> new_connection) {
    if (error)
        throw std::runtime_error(error.message());
    new_connection->start();
    start_async_receive(acceptor);
}

void start_async_receive(boost::asio::ip::tcp::acceptor &acceptor) {
    auto new_connection = std::make_shared<tcp_connection>(acceptor.get_io_context());
    acceptor.async_accept(new_connection->socket(),
                          std::bind(&handle_new_tcp_connection,
                                    std::placeholders::_1,
                                    std::ref(acceptor),
                                    new_connection)
    );
}

const unsigned port_number{5855};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage " << argv[0] << " server|client\n";
        return EXIT_FAILURE;
    }
    boost::asio::io_context io_context{BOOST_ASIO_CONCURRENCY_HINT_SAFE};
    auto work_guard = boost::asio::make_work_guard(io_context);
    auto worker = std::thread{[&io_context]() {
        while (true) {
            try {
                io_context.run();
                break;
            } catch (std::exception &e) {
                std::cerr << e.what() << "\n";
            }
        }
    }};

    const std::string pipe_name{"FooBar"};

    if (std::string(argv[1]) == "server") {
        boost::asio::ip::tcp::acceptor my_socket{io_context,
                                                 boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(),
                                                                                port_number}};
        start_async_receive(my_socket);
        for (std::string s; std::cin >> s; std::cout << "Type quit to quit\n") {
            if (s == "quit")
                break;
        }
    } else {
        boost::asio::ip::tcp::socket client_socket{io_context};
        client_socket.connect(boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), port_number});


        auto input = std::make_shared<std::array<char, 16>>();

        auto output = std::make_shared<std::string>();
        while (std::cin >> *output) {
            if (*output == "quit")
                break;
            std::cout << "Is open: " << client_socket.is_open() << std::endl;
            boost::asio::async_write(client_socket, boost::asio::buffer(*output),
                                     std::bind([](auto error, auto buffer) {
                                         if (error)
                                             std::cerr << error.message() << "\n";
                                         std::cout << "Sent " << *buffer << "\n";
                                     }, std::placeholders::_1, output));
            output = std::make_shared<std::string>();
        }
    }
    std::cout << "Quitting" << std::endl;

    work_guard.reset();
    io_context.stop();
    worker.join();

}

