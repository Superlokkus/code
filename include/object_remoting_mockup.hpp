/*! @file object_remoting_mockup.hpp
 *
 */

#ifndef MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
#define MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP

#include <string>


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
    virtual std::string example_method_1(std::string input1, int input2) = 0;

    /*! @brief Increases the example_member by 1
     *
     */
    virtual void example_method_2(void) noexcept = 0;

    unsigned example_member{42};
};

class client_stub : public interface {
    client_stub();

    std::string example_method_1(std::string input1, int input2) override;

    void example_method_2(void) noexcept override;
};

class server_stub : public interface {
    server_stub();

    std::string example_method_1(std::string input1, int input2) override;

    void example_method_2(void) noexcept override;
};

}
}


#endif //MKDT_INTERFACE_OBJECT_REMOTING_MOCKUP_HPP
