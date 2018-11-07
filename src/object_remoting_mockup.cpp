/*! @file object_remoting_mockup.cpp
 *
 */

#include <object_remoting_mockup.hpp>

#include <vector>
#include <cstdint>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

template<typename Message_Type>
struct rpc_message_base {
    static constexpr const char *const magic_message_start_text = u8"mkdt_object_remoting_mockup_protocol";
    static constexpr const unsigned version_major = 1;
    static constexpr const unsigned version_minor = 0;

    using parameter_type = boost::variant<std::string, unsigned>;

    static constexpr const char *const magic_message_end_text = u8"mkdt_end";
};

struct rpc_request : rpc_message_base<rpc_request> {
    static constexpr const char *const magic_rpc_request_phrase = u8"request";
    enum struct member_function_type {
        example_method_1,
        example_method_2,
        example_member_get,
        example_member_set,
    };

    member_function_type function_to_call;
    std::vector<parameter_type> parameters;
};

struct rpc_response : rpc_message_base<rpc_response> {
    static constexpr const char *const magic_rpc_reponse_phrase = u8"response";
    using exception_text = std::string;
    bool is_exception;
    /*! @brief Holds either the return value to the called function or a parameter for the exception, in both
     * cases optional
     *
     */
    boost::optional<parameter_type> return_value;
};

using rpc_message = boost::variant<rpc_request, rpc_response>;

BOOST_FUSION_ADAPT_STRUCT(
        rpc_request,
        (rpc_request::member_function_type, function_to_call)
                (std::vector<rpc_request::parameter_type>, parameters)
)

BOOST_FUSION_ADAPT_STRUCT(
        rpc_response,
        (bool, is_exception)
                (boost::optional<rpc_request::parameter_type>, return_value)
)

namespace ns = ::boost::spirit::standard;

template<typename Iterator, typename Message_Type>
struct common_rules {
    boost::spirit::qi::rule<Iterator, std::string()> quoted_string{
            ::boost::spirit::qi::lexeme['"' >> +(ns::char_ - ('"')) >> '"']};

    boost::spirit::qi::rule<Iterator, typename rpc_message_base<Message_Type>::parameter_type()> parameter_type_{
            boost::spirit::qi::uint_ | quoted_string};

};

template<typename Iterator>
struct rpc_request_grammar
        : ::boost::spirit::qi::grammar<Iterator, rpc_request()>,
          common_rules<Iterator, rpc_request> {

    struct member_function_type_rule
            : boost::spirit::qi::symbols<char, rpc_request::member_function_type> {
        member_function_type_rule() {
            this->add(u8"example_method_1", rpc_request::member_function_type::example_method_1)
                    (u8"example_method_2", rpc_request::member_function_type::example_method_2)
                    (u8"example_member_get", rpc_request::member_function_type::example_member_get)
                    (u8"example_member_set", rpc_request::member_function_type::example_member_set);
        }
    } member_function_type_;

    rpc_request_grammar() : rpc_request_grammar::base_type(start) {

        using namespace ::boost::spirit::qi;


        start %= lit(rpc_request::magic_message_start_text) >> omit[+ns::space]
                                                            >> omit[uint_parser<unsigned, 10, 1>()(
                                                                    rpc_request::version_major)]
                                                            >> lit(u8".")
                                                            >> omit[uint_parser<unsigned, 10, 1>()]
                                                            >> omit[+ns::space]
                                                            >> lit(rpc_request::magic_rpc_request_phrase)
                                                            >> omit[+ns::space]
                                                            >> member_function_type_
                                                            >> omit[+ns::space]
                                                            >> rpc_request_grammar::parameter_type_ % u8","
                                                            >> omit[*ns::space]
                                                            >> lit(rpc_request::magic_message_end_text);
    }

    boost::spirit::qi::rule<Iterator, rpc_request()> start;

};

template<typename Iterator>
struct rpc_response_grammar
        : ::boost::spirit::qi::grammar<Iterator, rpc_response()>,
          common_rules<Iterator, rpc_response> {

    rpc_response_grammar() : rpc_response_grammar::base_type(start) {

        using namespace ::boost::spirit::qi;


        start %= lit(rpc_response::magic_message_start_text) >> omit[+ns::space]
                                                             >> omit[uint_parser<unsigned, 10, 1>()(
                                                                     rpc_response::version_major)]
                                                             >> lit(u8".")
                                                             >> omit[uint_parser<unsigned, 10, 1>()]
                                                             >> omit[+ns::space]
                                                             >> lit(rpc_response::magic_rpc_reponse_phrase)
                                                             >> omit[+ns::space]
                                                             >> bool_
                                                             >> omit[+ns::space]
                                                             >> -rpc_response_grammar::parameter_type_
                                                             >> omit[*ns::space]
                                                             >> lit(rpc_request::magic_message_end_text);
    }

    boost::spirit::qi::rule<Iterator, rpc_response()> start;

};

template<typename Iterator>
struct rpc_message_grammar
        : ::boost::spirit::qi::grammar<Iterator, rpc_message()> {
    rpc_message_grammar() : rpc_message_grammar::base_type(start) {
        start %= request | response;
    }

    boost::spirit::qi::rule<Iterator, rpc_message()> start;
    rpc_request_grammar<Iterator> request;
    rpc_response_grammar<Iterator> response;
};

template<typename OutputIterator, typename Message_Type>
struct common_generators {
    boost::spirit::karma::rule<OutputIterator, std::string()> quoted_string{
            boost::spirit::karma::lit(u8"\"") << boost::spirit::karma::string
                                              << boost::spirit::karma::lit(u8"\"")};

    boost::spirit::karma::rule<OutputIterator, typename rpc_message_base<Message_Type>::parameter_type()> parameter_type_{
            boost::spirit::karma::uint_ | quoted_string};

};

template<typename OutputIterator>
struct generate_rpc_request_grammar : boost::spirit::karma::grammar<OutputIterator, rpc_request()>,
                                      common_generators<OutputIterator, rpc_request> {
    struct member_function_type_gen :
            boost::spirit::karma::symbols<rpc_request::member_function_type, std::string> {
        member_function_type_gen() {
            this->add(rpc_request::member_function_type::example_method_1, u8"example_method_1")
                    (rpc_request::member_function_type::example_method_2, u8"example_method_2")
                    (rpc_request::member_function_type::example_member_get, u8"example_member_get")
                    (rpc_request::member_function_type::example_member_set, u8"example_member_set");
        }
    } member_function_type_gen_;

    generate_rpc_request_grammar() : generate_rpc_request_grammar::base_type(start) {
        using namespace boost::spirit::karma;
        start = lit(rpc_request::magic_message_start_text) << u8" "
                                                           << omit[uint_(rpc_request::version_major)]
                                                           << u8"."
                                                           << omit[uint_(rpc_request::version_minor)]
                                                           << u8" "
                                                           << lit(rpc_request::magic_rpc_request_phrase)
                                                           << u8" "
                                                           << member_function_type_gen_
                                                           << u8" "
                                                           << generate_rpc_request_grammar::parameter_type_ % u8","
                                                           << lit(rpc_request::magic_message_end_text);

    }

    boost::spirit::karma::rule<OutputIterator, rpc_request()> start;
};

template<typename OutputIterator>
struct generate_rpc_response_grammar : boost::spirit::karma::grammar<OutputIterator, rpc_response()>,
                                       common_generators<OutputIterator, rpc_response> {

    generate_rpc_response_grammar() : generate_rpc_response_grammar::base_type(start) {
        using namespace boost::spirit::karma;
        start = lit(rpc_response::magic_message_start_text) << u8" "
                                                            << omit[uint_(rpc_response::version_major)]
                                                            << u8"."
                                                            << omit[uint_(rpc_response::version_minor)]
                                                            << u8" "
                                                            << lit(rpc_response::magic_rpc_reponse_phrase)
                                                            << u8" "
                                                            << bool_
                                                            << u8" "
                                                            << -generate_rpc_response_grammar::parameter_type_
                                                            << lit(rpc_response::magic_message_end_text);

    }

    boost::spirit::karma::rule<OutputIterator, rpc_response()> start;
};

template<typename OutputIterator>
struct generate_rpc_message_grammar : boost::spirit::karma::grammar<OutputIterator, rpc_message()> {

    generate_rpc_message_grammar() : generate_rpc_message_grammar::base_type(start) {
        start = response | request;
    }

    boost::spirit::karma::rule<OutputIterator, rpc_message()> start;
    boost::spirit::karma::rule<OutputIterator, rpc_response()> response;
    boost::spirit::karma::rule<OutputIterator, rpc_request()> request;
};

mkdt::object_remoting_mockup::client_stub::client_stub() {

}

std::string
mkdt::object_remoting_mockup::client_stub::example_method_1(const std::string &input1, unsigned int input2) {
    return std::string{};
}

void mkdt::object_remoting_mockup::client_stub::example_method_2(void) noexcept {

}


unsigned int mkdt::object_remoting_mockup::client_stub::example_member() noexcept {
    return 0;
}

void mkdt::object_remoting_mockup::client_stub::set_example_member(unsigned value) {

}

std::string mkdt::object_remoting_mockup::client_stub::handle_incoming_data(std::string input) {
    return std::string{};
}

void mkdt::object_remoting_mockup::client_stub::set_sending_callback(std::function<void(std::string)> send_callback) {
    this->send_callback_ = send_callback;
}

mkdt::object_remoting_mockup::server_stub::server_stub() : example_member_{1337} {

}

std::string
mkdt::object_remoting_mockup::server_stub::example_method_1(const std::string &input1, unsigned int input2) {
    std::string value{std::to_string(this->example_member())};
    value + input1;
    value + std::to_string(input2);

    if (input2 == 42)
        throw std::runtime_error{"Input2 was equal 42"};

    return value;
}

void mkdt::object_remoting_mockup::server_stub::example_method_2(void) noexcept {
    this->example_member_ += 1;
}

unsigned mkdt::object_remoting_mockup::server_stub::example_member() noexcept {
    return this->example_member_;
}

void mkdt::object_remoting_mockup::server_stub::set_example_member(unsigned value) {
    this->example_member_ = value;
}

std::string mkdt::object_remoting_mockup::server_stub::handle_incoming_data(std::string input) {
    return std::string{};
}

void mkdt::object_remoting_mockup::server_stub::set_sending_callback(std::function<void(std::string)> send_callback) {
    this->send_callback_ = send_callback;
}



