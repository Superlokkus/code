#define BOOST_TEST_MODULE Registry Test

#include <boost/test/included/unit_test.hpp>

#include <router_client.hpp>
#include <registry.hpp>

BOOST_AUTO_TEST_SUITE(registry_tests)

BOOST_AUTO_TEST_CASE(FIRST_TEST) {
    BOOST_CHECK(true);
    boost::asio::io_context io_context;
    mkdt::registry reg{io_context};
    mkdt::router_client registry{io_context};
}

BOOST_AUTO_TEST_SUITE_END()


