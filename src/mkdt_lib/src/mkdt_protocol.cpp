/*! @file mkdt_protocol.cpp
 *
 */

#include <mkdt_protocol.hpp>

template
struct mkdt::protocol::local_request_grammar<std::string::const_iterator>;

template
struct mkdt::protocol::generate_local_request_grammar<std::back_insert_iterator<std::string>>;
