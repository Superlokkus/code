#define BOOST_TEST_MODULE MKDT Protocol Test

#include <boost/test/included/unit_test.hpp>

#include <mkdt_protocol.hpp>

struct local_request_phrases_fixture {
    std::string expose_object_request{"mkdt/1  local_request  \t expose_object_message: \"Fo bar\t \\\"8\","
                                      "2BC69EAD-4ABA-4A39-92C0-9565F4D464B4  mkdt_local_message_end\r\n"};

    std::vector<std::string> invalid_stuff{
            {"RTSP/1.0\t200 \t  OK\r\n"},
            {"oijsdisdjlisdfjlrur93209p831§\"§=)§"},
            {"\n\r fajfajkj \n\n\r\r\n"},
            {"\r\n afsfas3244afs"},
            {" \r\n"},
            {""},
            {" "},
            {"jjlfsjflsjkl"},
            {"RTSP/1.0\t2000 \t  OK\r\n\r\n"},
            {"RTSP/1.0\t20 \t  OK\r\n\r\n"},
            {"RTSP/.0\t200 \t  OK\r\n\r\n"},
            {"RTSP/ \t200 \t  OK\r\n\r\n"}
    };
    std::string::const_iterator begin{};
    std::string::const_iterator end{};
    bool success{false};
    mkdt::protocol::local_request message{};

    void parse_phrase(const std::string &phrase) {
        mkdt::protocol::local_request_grammar<std::string::const_iterator> grammar{};
        begin = phrase.cbegin();
        end = phrase.cend();
        success = boost::spirit::qi::phrase_parse(begin, end, grammar,
                                                  boost::spirit::ascii::space, message);
    }
};

BOOST_FIXTURE_TEST_SUITE(local_request_phrases_tests, local_request_phrases_fixture)

BOOST_AUTO_TEST_CASE(expose_object_request_test) {
    parse_phrase(expose_object_request);
    BOOST_CHECK(success);
    BOOST_CHECK(begin == end);
    BOOST_REQUIRE_EQUAL(message.which(), 3);
    const auto &expose_request = boost::get<mkdt::protocol::expose_object_message>(message);
    BOOST_CHECK_EQUAL(expose_request.service_name, "Fo bar\t \"8");
    const std::array<uint8_t, 16> raw_uuid{0x2b, 0xc6, 0x9e, 0xad, 0x4a, 0xba, 0x4a, 0x39, 0x92, 0xc0, 0x95, 0x65, 0xf4,
                                           0xd4, 0x64, 0xb4};
    BOOST_TEST(expose_request.object == raw_uuid,
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_SUITE_END()
