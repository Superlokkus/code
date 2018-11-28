/*! @file mkdt_protocol.hpp
 *
 */
#ifndef MKDT_MKDT_PROTOCOL_HPP
#define MKDT_MKDT_PROTOCOL_HPP

#include <string>
#include <cstdint>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <common_definitions.hpp>

namespace mkdt {
namespace protocol {

using char_t = char;
using string = std::basic_string<char_t>;
namespace ns = ::boost::spirit::standard;

constexpr uint8_t major_version = 1;

struct register_service_message {
    service_identifier service_name;
}; //ok

struct unregister_service_message {
    service_identifier service_name;
}; //ok

struct use_service_request {
    service_identifier service_name;
}; //Response: obect_id

struct expose_object_message {
    service_identifier service_name;
    object_identifier object;
};//ok

struct consume_object_request {
    service_identifier service_name;
    object_identifier object;
};//youwon/nope

struct message_for_object {
    service_identifier service_name;
    object_identifier object;
    std::vector<uint8_t> message;
}; //Not a host for this service, or ok

using local_request = boost::variant<
        register_service_message, unregister_service_message,
        use_service_request, expose_object_message,
        consume_object_request, message_for_object
>;


template<typename Iterator>
struct local_request_grammar
        : ::boost::spirit::qi::grammar<Iterator, local_request()> {
    local_request_grammar() : local_request_grammar::base_type(start) {

        namespace qi = boost::spirit::qi;

        start %= qi::lit("mkdt/");
    }

    boost::spirit::qi::rule<Iterator, local_request()> start;

};
}
}

#endif //MKDT_MKDT_PROTOCOL_HPP
