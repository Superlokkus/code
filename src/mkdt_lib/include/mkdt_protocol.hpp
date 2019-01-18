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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <common_definitions.hpp>

namespace mkdt {
namespace protocol {

using char_t = char;
using string = std::basic_string<char_t>;
namespace ns = ::boost::spirit::standard;

constexpr uint8_t major_version = 1;

struct register_service_message {
    service_identifier service_name;

    register_service_message() = default;

    explicit register_service_message(service_identifier id) : service_name(std::move(id)) {}
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
}
}

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::expose_object_message,
        (mkdt::service_identifier, service_name)
                (mkdt::object_identifier, object)
)

namespace mkdt {
namespace protocol {

template<typename Iterator>
struct common_rules {
    boost::spirit::qi::rule<Iterator, boost::uuids::uuid()>
            uuid_internal_ = boost::spirit::qi::stream;
    boost::spirit::qi::rule<Iterator, boost::uuids::uuid()>
            uuid_{(boost::spirit::qi::lit("{") >> uuid_internal_ >> boost::spirit::qi::lit("}")) |
                  uuid_internal_};

    boost::spirit::qi::rule<Iterator, mkdt::protocol::string()>
            ctl{ns::cntrl};

    boost::spirit::qi::rule<Iterator, mkdt::protocol::string()> quoted_string{
            ::boost::spirit::qi::lexeme['"' >> +(ns::char_ - (ctl | '"')) >> '"']};

    boost::spirit::qi::rule<Iterator, register_service_message()>
            register_service_message_{quoted_string};

    boost::spirit::qi::rule<Iterator, expose_object_message()>
            expose_object_message_{boost::spirit::qi::lit("expose_object_message:")
                                           >> boost::spirit::qi::omit[+ns::space] >> quoted_string
                                           >> boost::spirit::qi::lit(",")
                                           >> uuid_};
};


template<typename Iterator>
struct local_request_grammar
        : ::boost::spirit::qi::grammar<Iterator, local_request()>, common_rules<Iterator> {
    local_request_grammar() : local_request_grammar::base_type(start) {

        namespace qi = boost::spirit::qi;

        start %= qi::lit("mkdt/") >> qi::omit[qi::uint_(major_version)]
                                  >> qi::omit[+ns::space] >> qi::lit("local_request") >> qi::omit[+ns::space]
                                  >> (common_rules<Iterator>::register_service_message_ |
                                      common_rules<Iterator>::expose_object_message_)
                                  >> qi::omit[+ns::space] >> qi::lit("mkdt_local_message_end\r\n");
    }

    boost::spirit::qi::rule<Iterator, local_request()> start;

};
}
}

#endif //MKDT_MKDT_PROTOCOL_HPP
