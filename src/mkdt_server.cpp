/*! @file mkdt_server.cpp
 *
 */

#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <boost/optional.hpp>

#include <router_server.hpp>
#include <common_definitions.hpp>

int main(int argc, char *argv[]) {
    boost::asio::io_context io_context{BOOST_ASIO_CONCURRENCY_HINT_SAFE};
    auto work_guard = boost::asio::make_work_guard(io_context);

    const auto thread_count{std::max<unsigned>(std::thread::hardware_concurrency(), 1)};
    std::vector<std::thread> io_run_threads_;
    std::generate_n(std::back_inserter(io_run_threads_),
                    thread_count, [&io_context]() {
                return std::thread{
                        [&io_context]() {
                            while (true) {
                                try {
                                    io_context.run();
                                    break;
                                } catch (std::exception &e) {
                                    std::cerr << e.what() << "\n";
                                }
                            }
                        }};
            });


    boost::optional<mkdt::router_server> server;

    if (argc == 1) {
        std::cout << "Starting on default port " << mkdt::mkdt_server_port_number << "\n";
        server = mkdt::router_server{io_context};
    } else if (argc == 2) {
        const uint16_t port = std::stoul(argv[1]);
        std::cout << "Start on on port " << port << "\n";
        server = mkdt::router_server{io_context, port};
    } else {
        std::cerr << "Usage " << argv[0] << "\n" << argv[0] << " <port>\n";
        throw std::runtime_error{"Usage error"};
    }

    for (std::string s; std::cin >> s; std::cout << "Type quit to quit\n") {
        if (s == "quit")
            break;
    }

    std::cout << "Quitting" << std::endl;
    work_guard.reset();
    io_context.stop();
    std::for_each(io_run_threads_.begin(), io_run_threads_.end(), [](auto &thread) {
        if (thread.joinable()) thread.join();
    });
}

