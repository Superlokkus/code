/*! @file object_remoting_mockup.hpp
 *
 */

#ifndef MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
#define MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP

#include <string>
#include <functional>
#include <future>
#include <vector>
#include <cstdint>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace mkdt {
namespace object_remoting_mockup {

class example_api {
public:
    virtual ~example_api() = default;

    /*! @brief Adds input2 at the end of input 1 and example_member as prefix and returns that string,
     * but also throws std::exception if input2 == 42
     *
     * @param input1 Example input
     * @param input2 Will be added to input1
     * @returns to_string(example_member) + input1 + to_string(input2)
     * @throws std::runtime_error if input2 == 42
     */
    virtual std::future<std::string> example_method_1(const std::string &input1, unsigned int input2) = 0;

    /*! @brief Increases the example_member by 1
     *
     */
    virtual std::future<void> example_method_2(void) noexcept = 0;

    virtual std::future<unsigned int> example_member() noexcept = 0;

    virtual std::future<void> set_example_member(unsigned value) = 0;
};

class example_api_rpc : public example_api {
public:
    virtual void handle_incoming_data(std::string input) = 0;
    virtual void set_sending_callback(std::function<void(std::string)> send_callback) = 0;
};

using parameter_type = boost::variant<std::string, unsigned>;

struct rpc_request {
    enum struct member_function_type {
        example_method_1,
        example_method_2,
        example_member_get,
        example_member_set,
    };

    member_function_type function_to_call;
    std::vector<parameter_type> parameters;
};

struct rpc_response {
    using exception_text = std::string;
    bool is_exception;
    /*! @brief Holds either the return value to the called function or a parameter for the exception, in both
     * cases optional
     *
     */
    boost::optional<parameter_type> return_value;
};

using rpc_message = boost::variant<rpc_request, rpc_response>;

class client_stub : public example_api_rpc {
public:
    client_stub();

    std::future<std::string> example_method_1(const std::string &input1, unsigned int input2) override;

    std::future<void> example_method_2(void) noexcept override;

    std::future<unsigned int> example_member() noexcept override;

    std::future<void> set_example_member(unsigned value) override;

    void handle_incoming_data(std::string input) override;

    void set_sending_callback(std::function<void(std::string)> send_callback) override;

private:
    std::function<void(std::string)> send_callback_;
    boost::optional<boost::variant<std::promise<void>, std::promise<unsigned>>> last_promise_;

};

class server_stub : public example_api_rpc {
public:
    server_stub();

    std::future<std::string> example_method_1(const std::string &input1, unsigned int input2) override;

    std::future<void> example_method_2(void) noexcept override;

    std::future<unsigned int> example_member() noexcept override;

    std::future<void> set_example_member(unsigned value) override;

    void handle_incoming_data(std::string input) override;

    void set_sending_callback(std::function<void(std::string)> send_callback) override;

private:
    unsigned example_member_;
    std::function<void(std::string)> send_callback_;
};

}
}


#endif //MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
