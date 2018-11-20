#define BOOST_TEST_MODULE Object remoting mockup test

#include <boost/test/included/unit_test.hpp>

#include <object_remoting_mockup.hpp>

BOOST_AUTO_TEST_SUITE(object_remoting_mockup_tests)


BOOST_AUTO_TEST_SUITE(client_stub_tests)

BOOST_AUTO_TEST_CASE(client_default_constructed_tests) {
    mkdt::object_remoting_mockup::client_stub client{};

    BOOST_CHECK_THROW(client.example_method_1("Foo", 5), std::bad_function_call);
}

BOOST_AUTO_TEST_CASE(client_example1_tests) {
    mkdt::object_remoting_mockup::client_stub client{};
    std::string output;
    client.set_sending_callback([&output](auto input) {
        output = input;
    });

    auto return_value = client.example_method_1("Foo", 5);

    BOOST_CHECK_EQUAL(output, "mkdt_object_remoting_mockup_protocol 1.0 request example_method_1 \"Foo\",5 mkdt_end");

    client.handle_incoming_data("mkdt_object_remoting_mockup_protocol 1.0 response false \"bar\" mkdt_end");

    BOOST_CHECK_EQUAL(return_value.get(), "bar");

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(server_stub_tests)

BOOST_AUTO_TEST_CASE(server_example1_tests) {
    mkdt::object_remoting_mockup::server_stub server{};

    auto return_value = server.example_method_1("Foo", 5);

    BOOST_CHECK_EQUAL(return_value.get(), "1337Foo5");

    return_value = server.example_method_1("Foo", 42);

    BOOST_CHECK_THROW(return_value.get(), std::exception);
}

BOOST_AUTO_TEST_SUITE_END()


struct integration_fixture {
    integration_fixture() {
        client.set_sending_callback(std::bind(&decltype(server)::handle_incoming_data, &server, std::placeholders::_1));
        server.set_sending_callback(std::bind(&decltype(client)::handle_incoming_data, &client, std::placeholders::_1));
    }

    mkdt::object_remoting_mockup::client_stub client;
    mkdt::object_remoting_mockup::server_stub server;

};

BOOST_FIXTURE_TEST_SUITE(integration_tests, integration_fixture)

BOOST_AUTO_TEST_CASE(example_method_1_tests) {
    auto return_value = client.example_method_1("Foo Bar!", 56);

    BOOST_CHECK_EQUAL(return_value.get(), "1337Foo Bar!56");

    return_value = client.example_method_1("Foo", 42);

    BOOST_CHECK_THROW(return_value.get(), std::exception);
}

BOOST_AUTO_TEST_CASE(example_member_tests) {
    BOOST_CHECK_EQUAL(client.example_member().get(), 1337);
    constexpr unsigned magic_number{3402392390u};
    client.set_example_member(magic_number).get();
    BOOST_CHECK_EQUAL(client.example_member().get(), magic_number);
}


BOOST_AUTO_TEST_CASE(example_method_2_tests) {
    BOOST_CHECK_EQUAL(client.example_member().get(), 1337);
    client.example_method_2().get();
    BOOST_CHECK_EQUAL(client.example_member().get(), 1338);
}

BOOST_AUTO_TEST_SUITE_END() //client_tests

BOOST_AUTO_TEST_SUITE_END()
