#define BOOST_TEST_MODULE Object remoting mockup test

#include <boost/test/included/unit_test.hpp>

#include <object_remoting_mockup.hpp>

BOOST_AUTO_TEST_SUITE(object_remoting_mockup_tests)

struct client_fixture {
    client_fixture() {
    }

    mkdt::object_remoting_mockup::client_stub default_constructed_client;
    mkdt::object_remoting_mockup::client_stub connected_client;
};

BOOST_FIXTURE_TEST_SUITE(client_tests, client_fixture)

BOOST_AUTO_TEST_CASE(client_stub_test) {
    mkdt::object_remoting_mockup::client_stub client{};


    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END() //client_tests

BOOST_AUTO_TEST_SUITE_END()
