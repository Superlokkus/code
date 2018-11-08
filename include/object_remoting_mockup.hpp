/*! @file object_remoting_mockup.hpp
 *
 */

#ifndef MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
#define MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP

#include <string>
#include <functional>
#include <future>


namespace mkdt {
namespace object_remoting_mockup {

class interface {
public:
    virtual ~interface() = default;

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

    //Extra interface
    virtual std::string handle_incoming_data(std::string input) = 0;

    virtual void set_sending_callback(std::function<void(std::string)> send_callback) = 0;
};

class client_stub : public interface {
public:
    client_stub();

    std::future<std::string> example_method_1(const std::string &input1, unsigned int input2) override;

    std::future<void> example_method_2(void) noexcept override;

    std::future<unsigned int> example_member() noexcept override;

    std::future<void> set_example_member(unsigned value) override;

    std::string handle_incoming_data(std::string input) override;

    void set_sending_callback(std::function<void(std::string)> send_callback) override;

private:
    std::function<void(std::string)> send_callback_;
};

class server_stub : public interface {
public:
    server_stub();

    std::future<std::string> example_method_1(const std::string &input1, unsigned int input2) override;

    std::future<void> example_method_2(void) noexcept override;

    std::future<unsigned int> example_member() noexcept override;

    std::future<void> set_example_member(unsigned value) override;

    std::string handle_incoming_data(std::string input) override;

    void set_sending_callback(std::function<void(std::string)> send_callback) override;

private:
    unsigned example_member_;
    std::function<void(std::string)> send_callback_;
};

}
}


#endif //MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
