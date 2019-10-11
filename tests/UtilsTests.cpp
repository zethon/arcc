#include <boost/test/unit_test.hpp>
#include "../arcc/SimpleArgs.h"
#include "../arcc/utils.h"

BOOST_AUTO_TEST_SUITE(arccutils)

BOOST_AUTO_TEST_CASE(SimpleArgs)
{
    using arcc::SimpleArgs;

    SimpleArgs args{ "a b c"};
    BOOST_CHECK_EQUAL(args.getTokenCount(), 3u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 3u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 0u);
    BOOST_CHECK_EQUAL(args.getPositional(0), "a");
    BOOST_CHECK_EQUAL(args.getPositional(1), "b");
    BOOST_CHECK_EQUAL(args.getPositional(2), "c");

    args.clear();
    args.parse("aa --param1=val1 --param2=val2");
    BOOST_CHECK_EQUAL(args.getTokenCount(), 5u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 1u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 2u);
    BOOST_CHECK_EQUAL(args.getPositional(0), "aa");
    BOOST_CHECK_EQUAL(args.getNamedArgument("param1"), "val1");
    BOOST_CHECK_EQUAL(args.getNamedArgument("param2"), "val2");

    args.clear();
    args.parse("--limit=10 --verbose --details");
    BOOST_CHECK_EQUAL(args.getTokenCount(), 4u);
    BOOST_CHECK(args.hasArgument("limit"));
    BOOST_CHECK_EQUAL(args.getNamedArgument("limit"), "10");
    BOOST_CHECK(args.hasArgument("verbose"));
    BOOST_CHECK(args.hasArgument("details"));

    args.clear();
    BOOST_CHECK_EQUAL(args.getTokenCount(), 0u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 0u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 0u);
}

BOOST_AUTO_TEST_CASE(convertToBool)
{
    BOOST_CHECK_EQUAL(utils::convertToBool("true"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("TRUE"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("on"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("ON"), true);

    BOOST_CHECK_EQUAL(utils::convertToBool("false"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("FALSE"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("FalSe"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("off"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("OFF"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("OfF"), false);
    
    BOOST_CHECK_NE(utils::convertToBool("true"), false);
    BOOST_CHECK_NE(utils::convertToBool("false"), true);
}

BOOST_AUTO_TEST_CASE(isNumeric)
{
    BOOST_CHECK_EQUAL(utils::isNumeric("0"), true);
    BOOST_CHECK_EQUAL(utils::isNumeric("123"), true);
    BOOST_CHECK_EQUAL(utils::isNumeric("cat"), false);
    BOOST_CHECK_EQUAL(utils::isNumeric("-0"), false);
}

BOOST_AUTO_TEST_CASE(isBoolean)
{
    BOOST_CHECK_EQUAL(utils::isBoolean("true"), true);
    BOOST_CHECK_EQUAL(utils::isBoolean("123"), false);
    BOOST_CHECK_EQUAL(utils::isBoolean("FALSE"), true);
    BOOST_CHECK_EQUAL(utils::isBoolean("tRue"), true);
}

BOOST_AUTO_TEST_SUITE_END()