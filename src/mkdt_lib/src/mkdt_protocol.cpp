/*! @file mkdt_protocol.cpp
 *
 */

#include <mkdt_protocol.hpp>

template
struct mkdt::protocol::local_request_grammar<std::string::const_iterator>;

template
struct mkdt::protocol::local_response_grammar<std::string::const_iterator>;

template
struct mkdt::protocol::local_message_grammar<std::string::const_iterator>;

template
struct mkdt::protocol::generate_local_request_grammar<std::back_insert_iterator<std::string>>;

template
struct mkdt::protocol::generate_local_response_grammar<std::back_insert_iterator<std::string>>;

template
struct mkdt::protocol::generate_local_message_grammar<std::back_insert_iterator<std::string>>;

mkdt::error mkdt::protocol::response_to_error(const mkdt::protocol::local_response &response) {
    mkdt::error error{};
    uint16_t code{200};
    std::string text;
    switch (response.which()) {
        case 0:
            code = boost::get<simple_confirm>(response).code;
            text = boost::get<simple_confirm>(response).text;
            break;
        case 1:
            code = boost::get<object_answer>(response).request_in_general.code;
            text = boost::get<object_answer>(response).request_in_general.text;
        default:
            break;
    }

    if (code >= 300 || code < 200) {
        error = mkdt::error{std::string{"Server response negative: "} + std::to_string(code) + " " + text};
    }

    return error;
}
