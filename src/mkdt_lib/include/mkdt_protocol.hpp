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
    string magic_prefix; //!< Constant, just for https://stackoverflow.com/a/19824426/3537677
    service_identifier service_name;
}; //ok

struct unregister_service_message {
    string magic_prefix; //!< Constant, just for https://stackoverflow.com/a/19824426/3537677
    service_identifier service_name;
}; //ok

struct use_service_request {
    string magic_prefix; //!< Constant, just for https://stackoverflow.com/a/19824426/3537677
    service_identifier service_name;
}; //Response: obect_id

struct expose_object_message {
    service_identifier service_name;
    object_identifier object{};
};//ok

struct consume_object_request {
    service_identifier service_name;
    object_identifier object{};
};//youwon/nope

struct message_for_object {
    service_identifier service_name;
    object_identifier receiver{};
    string message;
    boost::optional<object_identifier> sender;
}; //Not a host for this service, or ok

using local_request = boost::variant<
        register_service_message, unregister_service_message,
        use_service_request, expose_object_message,
        consume_object_request, message_for_object
>;

struct simple_confirm {
    uint16_t code;
    string text;
};

struct object_answer {
    simple_confirm request_in_general;
    object_identifier object{};
};

using local_response = boost::variant<simple_confirm, object_answer>;

using local_message = boost::variant<local_request, local_response>;

mkdt::error response_to_error(const local_response &response);

}
}

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::register_service_message,
        (mkdt::protocol::string, magic_prefix)
                (mkdt::service_identifier, service_name)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::unregister_service_message,
        (mkdt::protocol::string, magic_prefix)
                (mkdt::service_identifier, service_name)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::use_service_request,
        (mkdt::protocol::string, magic_prefix)
                (mkdt::service_identifier, service_name)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::expose_object_message,
        (mkdt::service_identifier, service_name)
                (mkdt::object_identifier, object)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::consume_object_request,
        (mkdt::service_identifier, service_name)
                (mkdt::object_identifier, object)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::message_for_object,
        (mkdt::service_identifier, service_name)
                (mkdt::object_identifier, receiver)
                (mkdt::protocol::string, message)
                (boost::optional<mkdt::object_identifier>, sender)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::simple_confirm,
        (uint16_t, code)
                (mkdt::protocol::string, text)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::protocol::object_answer,
        (mkdt::protocol::simple_confirm, request_in_general)
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

    boost::spirit::qi::rule<Iterator, mkdt::protocol::string()> quoted_string{
            ::boost::spirit::qi::lexeme['"'
                    >> +(boost::spirit::qi::as<mkdt::protocol::string>()[(ns::char_ - '"' - '\\')]
                         | ns::string("\\\"")) >> '"']};

    boost::spirit::qi::rule<Iterator, register_service_message()>
            register_service_message_{ns::string("register_service_message:")
                                              >> boost::spirit::qi::omit[+ns::space] >> quoted_string
    };

    boost::spirit::qi::rule<Iterator, unregister_service_message()>
            unregister_service_message_{ns::string("unregister_service_message:")
                                                >> boost::spirit::qi::omit[+ns::space] >> quoted_string
    };

    boost::spirit::qi::rule<Iterator, use_service_request()>
            use_service_request_{ns::string("use_service_request:")
                                         >> boost::spirit::qi::omit[+ns::space] >> quoted_string
    };

    boost::spirit::qi::rule<Iterator, expose_object_message()>
            expose_object_message_{boost::spirit::qi::lit("expose_object_message:")
                                           >> boost::spirit::qi::omit[+ns::space] >> quoted_string
                                           >> boost::spirit::qi::lit(",")
                                           >> uuid_};

    boost::spirit::qi::rule<Iterator, consume_object_request()>
            consume_object_request_{boost::spirit::qi::lit("consume_object_request:")
                                            >> boost::spirit::qi::omit[+ns::space] >> quoted_string
                                            >> boost::spirit::qi::lit(",")
                                            >> uuid_};

    boost::spirit::qi::rule<Iterator, message_for_object()>
            message_for_object_{boost::spirit::qi::lit("message_for_object:") >> boost::spirit::qi::omit[+ns::space]
                                                                              >> quoted_string
                                                                              >> boost::spirit::qi::lit(",")
                                                                              >> uuid_
                                                                              >> boost::spirit::qi::lit(",")
                                                                              >> quoted_string
                                                                              >> -(boost::spirit::qi::lit(",")
                                                                                      >> uuid_)};
    boost::spirit::qi::rule<Iterator, simple_confirm()>
            simple_confirm_{boost::spirit::qi::uint_parser<uint16_t, 10, 3>()
                                    >> boost::spirit::qi::omit[+ns::space] >> quoted_string};

    boost::spirit::qi::rule<Iterator, object_answer()>
            object_answer_{simple_confirm_ >> boost::spirit::qi::omit[+ns::space] >> uuid_};
};


template<typename Iterator>
struct local_request_grammar
        : ::boost::spirit::qi::grammar<Iterator, local_request()>, common_rules<Iterator> {
    local_request_grammar() : local_request_grammar::base_type(start) {

        namespace qi = boost::spirit::qi;

        start %= qi::lit("mkdt/") >> qi::omit[qi::uint_(major_version)]
                                  >> qi::omit[+ns::space] >> qi::lit("local_request") >> qi::omit[+ns::space]
                                  >> (common_rules<Iterator>::register_service_message_ |
                                      common_rules<Iterator>::unregister_service_message_ |
                                      common_rules<Iterator>::use_service_request_ |
                                      common_rules<Iterator>::expose_object_message_ |
                                      common_rules<Iterator>::consume_object_request_ |
                                      common_rules<Iterator>::message_for_object_
                                  )
                                  >> qi::omit[+ns::space] >> qi::lit("mkdt_local_message_end\r\n");
    }

    boost::spirit::qi::rule<Iterator, local_request()> start;

};

template<typename Iterator>
struct local_response_grammar
        : ::boost::spirit::qi::grammar<Iterator, local_response()>, common_rules<Iterator> {
    local_response_grammar() : local_response_grammar::base_type(start) {

        namespace qi = boost::spirit::qi;

        start %= qi::lit("mkdt/") >> qi::omit[qi::uint_(major_version)]
                                  >> qi::omit[+ns::space] >> qi::lit("local_response") >> qi::omit[+ns::space]
                                  >> (common_rules<Iterator>::object_answer_ |
                                      common_rules<Iterator>::simple_confirm_)
                                  >> qi::omit[+ns::space] >> qi::lit("mkdt_local_message_end\r\n");
    }

    boost::spirit::qi::rule<Iterator, local_response()> start;

};

template<typename Iterator>
struct local_message_grammar
        : ::boost::spirit::qi::grammar<Iterator, local_message()>, common_rules<Iterator> {
    local_message_grammar() : local_message_grammar::base_type(start) {

        namespace qi = boost::spirit::qi;

        start %= request | response;
    }

    local_request_grammar<Iterator> request;
    local_response_grammar<Iterator> response;
    boost::spirit::qi::rule<Iterator, local_message()> start;
};

template<typename OutputIterator>
struct common_generators {
    boost::spirit::karma::rule<OutputIterator, boost::uuids::uuid()>
            uuid_{
            boost::spirit::karma::stream
    };

    boost::spirit::karma::rule<OutputIterator, register_service_message()>
            register_service_message_{boost::spirit::skip[boost::spirit::karma::string] <<
                                                                                        boost::spirit::karma::lit(
                                                                                                "register_service_message: \"")
                                                                                        << boost::spirit::karma::string
                                                                                        << "\""
    };

    boost::spirit::karma::rule<OutputIterator, unregister_service_message()>
            unregister_service_message_{boost::spirit::skip[boost::spirit::karma::string] <<
                                                                                          boost::spirit::karma::lit(
                                                                                                  "unregister_service_message: \"")
                                                                                          << boost::spirit::karma::string
                                                                                          << "\""
    };

    boost::spirit::karma::rule<OutputIterator, use_service_request()>
            use_service_request_{boost::spirit::skip[boost::spirit::karma::string] <<
                                                                                   boost::spirit::karma::lit(
                                                                                           "use_service_request: \"")
                                                                                   << boost::spirit::karma::string
                                                                                   << "\""
    };

    boost::spirit::karma::rule<OutputIterator, expose_object_message()>
            expose_object_message_{
            boost::spirit::karma::lit("expose_object_message: \"")
                    << boost::spirit::karma::string
                    << "\"," << uuid_
    };

    boost::spirit::karma::rule<OutputIterator, consume_object_request()>
            consume_object_request_{
            boost::spirit::karma::lit("consume_object_request: \"")
                    << boost::spirit::karma::string
                    << "\"," << uuid_
    };

    boost::spirit::karma::rule<OutputIterator, message_for_object()>
            message_for_object_{
            boost::spirit::karma::lit("message_for_object: \"") << boost::spirit::karma::string
                                                                << boost::spirit::karma::lit("\",") << uuid_
                                                                << boost::spirit::karma::lit(",\"")
                                                                << boost::spirit::karma::string
                                                                << boost::spirit::karma::lit("\"")
                                                                << -("," << uuid_)
    };

    boost::spirit::karma::rule<OutputIterator, simple_confirm()>
            simple_confirm_{
            boost::spirit::karma::uint_ <<
                                        boost::spirit::karma::lit(" \"") << boost::spirit::karma::string
                                        << boost::spirit::karma::lit("\"")

    };

    boost::spirit::karma::rule<OutputIterator, object_answer()>
            object_answer_{
            simple_confirm_ << uuid_
    };

};

template<typename OutputIterator>
struct generate_local_request_grammar : boost::spirit::karma::grammar<OutputIterator, local_request()>,
                                        common_generators<OutputIterator> {
    generate_local_request_grammar() : generate_local_request_grammar::base_type(start) {
        namespace karma = boost::spirit::karma;
        start = karma::lit("mkdt/") << major_version_
                                    << karma::lit(" local_request ") << (
                                            common_generators<OutputIterator>::register_service_message_
                                            | common_generators<OutputIterator>::unregister_service_message_
                                            | common_generators<OutputIterator>::use_service_request_
                                            | common_generators<OutputIterator>::expose_object_message_
                                            | common_generators<OutputIterator>::consume_object_request_
                                            | common_generators<OutputIterator>::message_for_object_
                                    ) << karma::lit(" mkdt_local_message_end\r\n");
    }
    boost::spirit::karma::rule<OutputIterator, local_request()> start;
    boost::spirit::karma::rule<OutputIterator, boost::spirit::karma::unused_type()> major_version_{
            boost::spirit::karma::uint_(major_version)
    };
};

template<typename OutputIterator>
struct generate_local_response_grammar : boost::spirit::karma::grammar<OutputIterator, local_response()>,
                                         common_generators<OutputIterator> {
    generate_local_response_grammar() : generate_local_response_grammar::base_type(start) {
        namespace karma = boost::spirit::karma;
        start = karma::lit("mkdt/") << major_version_
                                    << karma::lit(" local_response ") << (
                                            common_generators<OutputIterator>::object_answer_
                                            | common_generators<OutputIterator>::unregister_service_message_
                                            | common_generators<OutputIterator>::simple_confirm_
                                    ) << karma::lit(" mkdt_local_message_end\r\n");

    }
    boost::spirit::karma::rule<OutputIterator, local_response()> start;
    boost::spirit::karma::rule<OutputIterator, boost::spirit::karma::unused_type()> major_version_{
            boost::spirit::karma::uint_(major_version)
    };
};

template<typename OutputIterator>
struct generate_local_message_grammar : boost::spirit::karma::grammar<OutputIterator, local_message()>,
                                        common_generators<OutputIterator> {
    generate_local_message_grammar() : generate_local_message_grammar::base_type(start) {
        namespace karma = boost::spirit::karma;
        start = request | response;

    }

    boost::spirit::karma::rule<OutputIterator, local_message()> start;
    generate_local_request_grammar<OutputIterator> request;
    generate_local_response_grammar<OutputIterator> response;
};

}
}

#endif //MKDT_MKDT_PROTOCOL_HPP
