#define BOOST_TEST_MODULE MKDT Protocol Test

#include <boost/test/included/unit_test.hpp>

#include <mkdt_protocol.hpp>

BOOST_AUTO_TEST_SUITE(mkdt_protocol_test)

struct local_request_phrases_fixture {
    std::string expose_object_request{"mkdt/1  local_request  \t expose_object_message: \"Fo bar\t \\\"8\","
                                      "2BC69EAD-4ABA-4A39-92C0-9565F4D464B4  mkdt_local_message_end\r\n"};
    std::string register_service_request{
            "mkdt/1 local_request register_service_message: \"FooBar\" mkdt_local_message_end\r\n"};

    std::string register_service_text{"register_service_message: \"FooBar\""};
    std::string expose_object_text{"expose_object_message: \"Fo bar\t \\\"8\","
                                   "2BC69EAD-4ABA-4A39-92C0-9565F4D464B4"};

    std::string message_for_object_request{"mkdt/1  local_request  \t "
                                           "message_for_object:  \"Service Name\","
                                           "2BC69EAD-4ABA-4A39-92C0-9565F4D464B5,"
                                           "\" 42 example message\","
                                           "2BC69EAD-4ABA-4A39-92C0-9565F4D464BF"
                                           " mkdt_local_message_end\r\n"};

    std::string message_for_object_without_sender_request{"mkdt/1  local_request  \t "
                                                          "message_for_object:  \"Service Name\","
                                                          "2BC69EAD-4ABA-4A39-92C0-9565F4D464B5,"
                                                          "\" 42 example message\""
                                                          " mkdt_local_message_end\r\n"};

    std::vector<std::string> invalid_stuff{
            {"mkdt/2 local_request register_service_message: \"FooBar\" mkdt_local_message_end\r\n"},
            {"mkdt_local_message_end\r\n"},
            {"RTSP/1.0\t200 \t  OK\r\n"},
            {"oijsdisdjlisdfjlrur93209p831§\"§=)§"},
            {"\n\r fajfajkj \n\n\r\r\n"},
            {"\r\n afsfas3244afs"},
            {" \r\n"},
            {""},
            {" "},
            {"jjlfsjflsjkl"},
    };
    std::string::const_iterator begin{};
    std::string::const_iterator end{};
    bool success{false};

    template<typename GrammarT, typename AttributeT>
    void parse_phrase(const std::string &phrase, GrammarT &grammar, AttributeT &attribute) {
        begin = phrase.cbegin();
        end = phrase.cend();
        success = boost::spirit::qi::parse(begin, end, grammar, attribute);

        BOOST_CHECK(success);
        BOOST_CHECK(begin == end);
        std::string parse_remainder{begin, end};
        BOOST_TEST_MESSAGE(parse_remainder);
    }
};

BOOST_FIXTURE_TEST_SUITE(local_request_phrases_tests, local_request_phrases_fixture)

BOOST_AUTO_TEST_CASE(register_service_rule_test) {
    mkdt::protocol::common_rules<std::string::const_iterator> rules{};
    mkdt::protocol::register_service_message message{};
    parse_phrase(register_service_text, rules.register_service_message_, message);
    BOOST_CHECK_EQUAL(message.service_name, "FooBar");
};

BOOST_AUTO_TEST_CASE(expose_object_rule_test) {
    mkdt::protocol::common_rules<std::string::const_iterator> rules{};
    mkdt::protocol::expose_object_message expose_request{};
    parse_phrase(expose_object_text, rules.expose_object_message_, expose_request);
    BOOST_CHECK_EQUAL(expose_request.service_name, "Fo bar\t \\\"8");
    const std::array<uint8_t, 16> raw_uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                                           0xd4, 0x64, 0xb4};
    BOOST_TEST(expose_request.object == raw_uuid,
               boost::test_tools::per_element());
};

BOOST_AUTO_TEST_CASE(expose_object_request_test) {
    mkdt::protocol::local_request_grammar<std::string::const_iterator> grammar{};
    mkdt::protocol::local_request message{};
    parse_phrase(expose_object_request, grammar, message);

    BOOST_REQUIRE_EQUAL(message.which(), 3);
    const auto &expose_request = boost::get<mkdt::protocol::expose_object_message>(message);
    BOOST_CHECK_EQUAL(expose_request.service_name, "Fo bar\t \\\"8");
    const std::array<uint8_t, 16> raw_uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                                           0xd4, 0x64, 0xb4};
    BOOST_TEST(expose_request.object == raw_uuid,
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(message_for_object_request_test) {
    mkdt::protocol::local_request_grammar<std::string::const_iterator> grammar{};
    mkdt::protocol::local_request message{};
    parse_phrase(message_for_object_request, grammar, message);

    BOOST_REQUIRE_EQUAL(message.which(), 5);
    const auto &object_message = boost::get<mkdt::protocol::message_for_object>(message);
    BOOST_CHECK_EQUAL(object_message.service_name, "Service Name");
    const std::array<uint8_t, 16> raw_uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                                           0xd4, 0x64, 0xb5};
    BOOST_CHECK(!object_message.receiver.is_nil());
    BOOST_TEST(object_message.receiver == raw_uuid,
               boost::test_tools::per_element());

    BOOST_CHECK_EQUAL(object_message.message, " 42 example message");

    const std::array<uint8_t, 16> raw_uuid2{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65,
                                            0xf4,
                                            0xd4, 0x64, 0xbf};
    BOOST_REQUIRE(object_message.sender);
    BOOST_CHECK(!object_message.sender->is_nil());
    BOOST_TEST(*object_message.sender == raw_uuid2,
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(message_for_object_without_sender_request_test) {
    mkdt::protocol::local_request_grammar<std::string::const_iterator> grammar{};
    mkdt::protocol::local_request message{};
    parse_phrase(message_for_object_without_sender_request, grammar, message);

    BOOST_REQUIRE_EQUAL(message.which(), 5);
    const auto &object_message = boost::get<mkdt::protocol::message_for_object>(message);
    BOOST_CHECK_EQUAL(object_message.service_name, "Service Name");
    const std::array<uint8_t, 16> raw_uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                                           0xd4, 0x64, 0xb5};
    BOOST_TEST(object_message.receiver == raw_uuid,
               boost::test_tools::per_element());

    BOOST_CHECK_EQUAL(object_message.message, " 42 example message");


    BOOST_CHECK(!object_message.sender);
}


BOOST_AUTO_TEST_CASE(register_service_request_test) {
    mkdt::protocol::local_request_grammar<std::string::const_iterator> grammar{};
    mkdt::protocol::local_request message{};
    parse_phrase(register_service_request, grammar, message);

    BOOST_REQUIRE_EQUAL(message.which(), 0);
    const auto &service_request = boost::get<mkdt::protocol::register_service_message>(message);
    BOOST_CHECK_EQUAL(service_request.service_name, "FooBar");

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(use_service_request_gen_test) {
    mkdt::protocol::local_request request{mkdt::protocol::use_service_request{{}, "My Foobar Service"}};
    std::string output;
    mkdt::protocol::generate_local_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(output,
                      "mkdt/1 local_request use_service_request: \"My Foobar Service\" mkdt_local_message_end\r\n");
}

BOOST_AUTO_TEST_CASE(message_for_object_gen_test) {
    mkdt::protocol::local_request request{mkdt::protocol::message_for_object{
            "My Service Name",
            boost::uuids::uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                               0xd4, 0x64, 0xbb},
            "Thats my 42 message",
            boost::uuids::uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                               0xd4, 0x64, 0xaf}
    }};
    std::string output;
    mkdt::protocol::generate_local_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(output,
                      "mkdt/1 local_request message_for_object: \"My Service Name\""
                      ",2bc69ead-4aba-4a39-92c0-9565f4d464bb,\"Thats my 42 message\""
                      ",2bc69ead-4aba-4a39-92c0-9565f4d464af"
                      " mkdt_local_message_end\r\n");
}

BOOST_AUTO_TEST_CASE(message_for_object_without_sender_gen_test) {
    mkdt::protocol::local_request request{mkdt::protocol::message_for_object{
            "My Service Name",
            boost::uuids::uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                               0xd4, 0x64, 0xbb},
            "Thats my 42 message",
            {}
    }};
    std::string output;
    mkdt::protocol::generate_local_request_grammar<std::back_insert_iterator<std::string>> gen_grammar{};
    const bool success = boost::spirit::karma::generate(std::back_inserter(output), gen_grammar, request);
    BOOST_CHECK(success);
    BOOST_CHECK_EQUAL(output,
                      "mkdt/1 local_request message_for_object: \"My Service Name\""
                      ",2bc69ead-4aba-4a39-92c0-9565f4d464bb,\"Thats my 42 message\""
                      " mkdt_local_message_end\r\n");
}

BOOST_AUTO_TEST_SUITE_END()
