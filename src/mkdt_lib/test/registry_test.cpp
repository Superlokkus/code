#define BOOST_TEST_MODULE Registry Test

#include <boost/test/included/unit_test.hpp>

#include <registry.hpp>
#include <object_remoting_mockup.hpp>

BOOST_AUTO_TEST_SUITE(registry_tests)

BOOST_AUTO_TEST_CASE(FIRST_TEST) {
    mkdt::object_remoting_mockup::client_stub client{};
    mkdt::object_remoting_mockup::server_stub server{};


    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()


