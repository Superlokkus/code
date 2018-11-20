/*! @file object_remoting_mockup.cpp
 *
 */

#include <object_remoting_mockup.hpp>

#include <stdexcept>
#include <iterator>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::object_remoting_mockup::rpc_request,
        (mkdt::object_remoting_mockup::rpc_request::member_function_type, function_to_call)
                (std::vector<mkdt::object_remoting_mockup::parameter_type>, parameters)
)

BOOST_FUSION_ADAPT_STRUCT(
        mkdt::object_remoting_mockup::rpc_response,
        (bool, is_exception)
                (boost::optional<mkdt::object_remoting_mockup::parameter_type>, return_value)
)

namespace mkdt {
namespace object_remoting_mockup {

namespace ns = ::boost::spirit::standard;

template<typename Iterator>
struct common_rules {
    boost::spirit::qi::rule<Iterator, std::string()> quoted_string{
            ::boost::spirit::qi::lexeme['"' >> +(ns::char_ - ('"')) >> '"']};

    boost::spirit::qi::rule<Iterator, parameter_type()> parameter_type_{
            boost::spirit::qi::uint_ | quoted_string};

};

template<typename Iterator>
struct rpc_request_grammar
        : ::boost::spirit::qi::grammar<Iterator, rpc_request()>,
          common_rules<Iterator> {

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


        start %= lit(u8"mkdt_object_remoting_mockup_protocol") >> omit[+ns::space]
                                                               >> omit[uint_parser<unsigned, 10, 1>()(1)]
                                                               >> lit(u8".")
                                                               >> omit[uint_parser<unsigned, 10, 1>()]
                                                               >> omit[+ns::space]
                                                               >> lit(u8"request")
                                                               >> omit[+ns::space]
                                                               >> member_function_type_
                                                               >> omit[+ns::space]
                                                               >> -(common_rules<Iterator>::parameter_type_ % u8",")
                                                               >> omit[*ns::space]
                                                               >> lit(u8"mkdt_end");
    }

    boost::spirit::qi::rule<Iterator, rpc_request()> start;

};

template<typename Iterator>
struct rpc_response_grammar
        : ::boost::spirit::qi::grammar<Iterator, rpc_response()>,
          common_rules<Iterator> {

    rpc_response_grammar() : rpc_response_grammar::base_type(start) {

        using namespace ::boost::spirit::qi;


        start %= lit(u8"mkdt_object_remoting_mockup_protocol") >> omit[+ns::space]
                                                               >> omit[uint_parser<unsigned, 10, 1>()(1)]
                                                               >> lit(u8".")
                                                               >> omit[uint_parser<unsigned, 10, 1>()]
                                                               >> omit[+ns::space]
                                                               >> lit(u8"response")
                                                               >> omit[+ns::space]
                                                               >> bool_
                                                               >> omit[+ns::space]
                                                               >> -common_rules<Iterator>::parameter_type_
                                                               >> omit[*ns::space]
                                                               >> lit(u8"mkdt_end");
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

template<typename OutputIterator>
struct common_generators {
    boost::spirit::karma::rule<OutputIterator, std::string()> quoted_string{
            boost::spirit::karma::lit(u8"\"") << boost::spirit::karma::string
                                              << boost::spirit::karma::lit(u8"\"")};

    boost::spirit::karma::rule<OutputIterator, parameter_type()> parameter_type_{
            boost::spirit::karma::uint_ | quoted_string};

};

template<typename OutputIterator>
struct generate_rpc_request_grammar : boost::spirit::karma::grammar<OutputIterator, rpc_request()>,
                                      common_generators<OutputIterator> {
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
        start %= lit(u8"mkdt_object_remoting_mockup_protocol") << lit(u8" ")
                                                               << lit(u8"1")
                                                               << lit(u8".")
                                                               << lit(u8"0")
                                                               << lit(u8" ")
                << lit(u8"request")
                << lit(u8" ")
                << member_function_type_gen_
                << lit(u8" ")
                << -(common_generators<OutputIterator>::parameter_type_ %
                                                                    u8",")
                << lit(u8" ")
                << lit(u8"mkdt_end");

    }

    boost::spirit::karma::rule<OutputIterator, rpc_request()> start;
};

template<typename OutputIterator>
struct generate_rpc_response_grammar : boost::spirit::karma::grammar<OutputIterator, rpc_response()>,
                                       common_generators<OutputIterator> {

    generate_rpc_response_grammar() : generate_rpc_response_grammar::base_type(start) {
        using namespace boost::spirit::karma;
        start = lit(u8"mkdt_object_remoting_mockup_protocol") << lit(u8" ")
                                                              << lit(u8"1")
                                                              << lit(u8".")
                << lit(u8"0")
                << lit(u8" ")
                << lit(u8"response")
                << lit(u8" ")
                << bool_
                << lit(u8" ")
                << -common_generators<OutputIterator>::parameter_type_
                << lit(u8" ")
                << lit(u8"mkdt_end");

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


template
struct rpc_request_grammar<std::string::const_iterator>;
template
struct rpc_response_grammar<std::string::const_iterator>;
template
struct generate_rpc_request_grammar<std::back_insert_iterator<std::string>>;
template
struct generate_rpc_response_grammar<std::back_insert_iterator<std::string>>;

}
}


mkdt::object_remoting_mockup::client_stub::client_stub() {

}

std::future<std::string>
mkdt::object_remoting_mockup::client_stub::example_method_1(const std::string &input1, unsigned int input2) {
    rpc_request request;

    request.function_to_call = rpc_request::member_function_type::example_method_1;
    request.parameters.emplace_back(std::string{input1});
    request.parameters.emplace_back(input2);

    std::string output;
    generate_rpc_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    if (!success)
        throw std::runtime_error("Request generation failed");

    std::promise<std::string> promise;
    auto future = promise.get_future();
    this->last_promise_ = std::move(promise);

    this->send_callback_(output);

    return future;
}

std::future<void> mkdt::object_remoting_mockup::client_stub::example_method_2(void) {
    rpc_request request;

    request.function_to_call = rpc_request::member_function_type::example_method_2;

    std::string output;
    generate_rpc_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    if (!success)
        throw std::runtime_error("Request generation failed");

    std::promise<void> promise;
    auto future = promise.get_future();
    this->last_promise_ = std::move(promise);

    this->send_callback_(output);

    return future;
}


std::future<unsigned int> mkdt::object_remoting_mockup::client_stub::example_member() {
    rpc_request request;

    request.function_to_call = rpc_request::member_function_type::example_member_get;

    std::string output;
    generate_rpc_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    if (!success)
        throw std::runtime_error("Request generation failed");

    std::promise<unsigned> promise;
    auto future = promise.get_future();
    this->last_promise_ = std::move(promise);

    this->send_callback_(output);

    return future;
}

std::future<void> mkdt::object_remoting_mockup::client_stub::set_example_member(unsigned value) {
    rpc_request request;

    request.function_to_call = rpc_request::member_function_type::example_member_set;

    request.parameters.emplace_back(value);
    std::string output;
    generate_rpc_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    if (!success)
        throw std::runtime_error("Request generation failed");

    std::promise<void> promise;
    auto future = promise.get_future();
    this->last_promise_ = std::move(promise);

    this->send_callback_(output);

    return future;
}

void mkdt::object_remoting_mockup::client_stub::handle_incoming_data(std::string input) {
    rpc_response_grammar<std::string::const_iterator> grammar{};
    rpc_response response;
    auto begin = input.cbegin();
    auto end = input.cend();
    auto parse_success = boost::spirit::qi::phrase_parse(begin, end, grammar,
                                                         boost::spirit::standard::space, response);
    auto &&last_promise{this->last_promise_.value()};
    if (last_promise.which() == 0) {
        auto &&promise = boost::get<std::promise<void>>(last_promise);
        try {
            if (parse_success && !response.is_exception) {
                promise.set_value();
            } else if (!parse_success) {
                throw std::runtime_error(std::string{"Repsonse malformed: "} +
                                         std::string{begin, end});
            } else {
                throw std::runtime_error(boost::get<std::string>(
                        response.return_value.value_or(
                                parameter_type{std::string{"Exception returned"}})));
            }
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    } else if (last_promise.which() == 1) {
        auto &&promise = boost::get<std::promise<unsigned>>(last_promise);
        try {
            if (parse_success && !response.is_exception) {
                promise.set_value(boost::get<unsigned>(response.return_value.value()));
            } else if (!parse_success) {
                throw std::runtime_error(std::string{"Repsonse malformed: "} +
                                         std::string{begin, end});
            } else {
                throw std::runtime_error(boost::get<std::string>(
                        response.return_value.value_or(
                                parameter_type{std::string{"Exception returned"}})));
            }
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    } else if (last_promise.which() == 2) {
        auto &&promise = boost::get<std::promise<std::string>>(last_promise);
        try {
            if (parse_success && !response.is_exception) {
                promise.set_value(boost::get<std::string>(response.return_value.value()));
            } else if (!parse_success) {
                throw std::runtime_error(std::string{"Repsonse malformed: "} +
                                         std::string{begin, end});
            } else {
                throw std::runtime_error(boost::get<std::string>(
                        response.return_value.value_or(
                                parameter_type{std::string{"Exception returned"}})));
            }
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    }
}

void mkdt::object_remoting_mockup::client_stub::set_sending_callback(std::function<void(std::string)> send_callback) {
    this->send_callback_ = send_callback;
}

mkdt::object_remoting_mockup::server_stub::server_stub() : example_member_{1337} {

}

std::future<std::string>
mkdt::object_remoting_mockup::server_stub::example_method_1(const std::string &input1, unsigned int input2) {
    std::packaged_task<std::string()> task([&]() {
        std::string value{std::to_string(this->example_member_)};
        value += input1;
        value += std::to_string(input2);

        if (input2 == 42)
            throw std::runtime_error{"Input2 was equal 42"};

        return value;
    });
    task();
    return task.get_future();
}

std::future<void> mkdt::object_remoting_mockup::server_stub::example_method_2(void) {
    std::promise<void> promise;
    promise.set_value();
    this->example_member_ += 1;
    return promise.get_future();
}

std::future<unsigned int> mkdt::object_remoting_mockup::server_stub::example_member() {
    std::promise<unsigned> value;
    value.set_value(this->example_member_);
    return value.get_future();
}

std::future<void> mkdt::object_remoting_mockup::server_stub::set_example_member(unsigned value) {
    std::promise<void> return_promise;
    return_promise.set_value();
    this->example_member_ = value;
    return return_promise.get_future();
}

void mkdt::object_remoting_mockup::server_stub::handle_incoming_data(std::string input) {
    rpc_request_grammar<std::string::const_iterator> grammar{};
    rpc_request request;
    auto begin = input.cbegin();
    auto end = input.cend();
    auto success = boost::spirit::qi::phrase_parse(begin, end, grammar,
                                                   boost::spirit::standard::space, request);
    if (!success)
        throw std::runtime_error(std::string{"Request not readable after"} + std::string{begin, end});

    rpc_response response;

    try {
        using mt = rpc_request::member_function_type;
        if (request.function_to_call == mt::example_method_1) {
            auto future = this->example_method_1(boost::get<std::string>(request.parameters.at(0)),
                                                 boost::get<unsigned>(request.parameters.at(1)));

            response.return_value = future.get();

        } else if (request.function_to_call == mt::example_method_2) {
            this->example_method_2().get();
        } else if (request.function_to_call == mt::example_member_set) {
            this->set_example_member(boost::get<unsigned>(request.parameters.at(0))).get();
        } else if (request.function_to_call == mt::example_member_get) {
            auto future = this->example_member();
            response.return_value = future.get();
        }
    } catch (const std::exception &e) {
        response.is_exception = true;
        response.return_value = e.what();
    }

    std::string output;
    generate_rpc_response_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, response);
    if (!success)
        throw std::runtime_error("Request generation failed");

    this->send_callback_(output);
}

void mkdt::object_remoting_mockup::server_stub::set_sending_callback(std::function<void(std::string)> send_callback) {
    this->send_callback_ = send_callback;
}



