/*! @file mkdt_example_client.cpp
 *
 */

#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include <mkdt_lib.hpp>
#include <object_remoting_mockup_adapter.hpp>


const char *const menu_text{R"END_MENU_TEXT(
Menu:
=====
quit
----------
register service <name>
use service <name>
----------
create object
destroy object <id>
expose object <id>
consume object <uid>
---------
object <id> example_method_1 <word> <number>
)END_MENU_TEXT"};

struct example_client {

    example_client(uint16_t port) :
            io_context_{BOOST_ASIO_CONCURRENCY_HINT_SAFE},
            work_guard_{boost::asio::make_work_guard(io_context_)},
            registry_(io_context_, port) {

        const auto thread_count{std::max<unsigned>(std::thread::hardware_concurrency(), 1)};

        std::generate_n(std::back_inserter(this->io_run_threads_),
                        thread_count,
                        [this]() {
                            return std::thread{&example_client::io_run_loop,
                                               std::ref(this->io_context_)};
                        });

    }

    ~example_client() {
        work_guard_.reset();
        io_context_.stop();
        std::for_each(this->io_run_threads_.begin(), this->io_run_threads_.end(), [](auto &thread) {
            if (thread.joinable()) thread.join();
        });
    }

    void connect() {

    }

    void disconnect() {

    }

    void register_service(mkdt::service_identifier name) {
        BOOST_LOG_TRIVIAL(info) << "Register  service \"" << name << "\"";
        auto service_object = std::make_shared<mkdt::object_remoting_mockup::
        stub_adapter<mkdt::object_remoting_mockup::server_stub>>(this->registry_);
        this->registry_.register_service(name, service_object, [=](auto error) {
            if (error)
                BOOST_LOG_TRIVIAL(error) << "Error while registering: " << error.what();
            else
                BOOST_LOG_TRIVIAL(info) << "Registered service " << name;
        });
    }

    void use_service(mkdt::service_identifier name) {
        BOOST_LOG_TRIVIAL(info) << "Use  service \"" << name << "\"";
        this->registry_.use_service_interface(name, [name](auto error, auto object_identifier) {
            if (error)
                BOOST_LOG_TRIVIAL(error) << "Error trying to use service " << name << " : " << error.what();
            else
                BOOST_LOG_TRIVIAL(info) << "Got object identifier \"" << boost::uuids::to_string(object_identifier)
                                    << "\" for service " << name;
        });
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
    std::vector<std::thread> io_run_threads_;
    mkdt::registry registry_;

    static void io_run_loop(boost::asio::io_context &context) {
        while (true) {
            try {
                context.run();
                break;
            } catch (std::exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
            }
        }

    }
};

int main(int argc, char *argv[]) {
    BOOST_LOG_TRIVIAL(info) << "MKDT example client\n";
    uint16_t port{mkdt::mkdt_server_port_number};
    if (argc == 1) {
        BOOST_LOG_TRIVIAL(info) << "Starting on default port " << mkdt::mkdt_server_port_number << "\n";
    } else if (argc == 2) {
        port = std::stoul(argv[1]);
        BOOST_LOG_TRIVIAL(info) << "Start on on port " << port << "\n";
    } else {
        std::cerr << "Usage " << argv[0] << "\n" << argv[0] << " <port>\n";
        throw std::runtime_error{"Usage error"};
    }

    example_client client{port};

    std::cout << menu_text << "Input: ";
    for (std::string input; std::getline(std::cin, input); std::cout << "Input: ") {
        if (input == "quit")
            break;
        if (input == "disonnect")
            client.disconnect();
        if (input == "reconnect") {
            client.disconnect();
            client.connect();
        }
        std::string input_prefix = "register service ";
        if (input.find(input_prefix) != std::string::npos) {
            client.register_service(input.substr(input.find(input_prefix) + input_prefix.size()));
        }

        input_prefix = "use service ";
        if (input.find(input_prefix) != std::string::npos) {
            client.use_service(input.substr(input.find(input_prefix) + input_prefix.size()));
        }
    }

}
