#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "../arcc/SimpleArgs.h"
#include "../arcc/CommandHistory.h"
#include "../arcc/utils.h"
#include "../arcc/Settings.h"

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(arccutils)

BOOST_AUTO_TEST_CASE(CommandHistory)
{
    using arcc::CommandHistory;

    CommandHistory history;

    history.commit("command 1");
    history.commit("command 2");
    history.commit("command 3");


    BOOST_CHECK_EQUAL(history.size(), 3u);
    BOOST_CHECK_EQUAL(history.up(), true);
    BOOST_CHECK_EQUAL(history.getCurrent(), "command 3");
    BOOST_CHECK_EQUAL(history.up(), true);
    BOOST_CHECK_EQUAL(history.getCurrent(), "command 2");
    BOOST_CHECK_EQUAL(history.up(), true);
    BOOST_CHECK_EQUAL(history.getCurrent(), "command 1");
    BOOST_CHECK_EQUAL(history.up(), false);

    history.reset();
    BOOST_CHECK_EQUAL(history.getCurrent(), "");
    BOOST_CHECK_EQUAL(history.up(), true);
    BOOST_CHECK_EQUAL(history.getCurrent(), "command 3");

    history.clear();
    BOOST_CHECK_EQUAL(history.size(), 0u);
}

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
    BOOST_CHECK_EQUAL(args.getTokenCount(), 3u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 1u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 2u);
    BOOST_CHECK_EQUAL(args.getPositional(0), "aa");
    BOOST_CHECK_EQUAL(args.getNamedArgument("param1"), "val1");
    BOOST_CHECK_EQUAL(args.getNamedArgument("param2"), "val2");

    args.clear();
    args.parse("--limit=10 --verbose --details");
    BOOST_CHECK_EQUAL(args.getTokenCount(), 3u);
    BOOST_CHECK(args.hasArgument("limit"));
    BOOST_CHECK_EQUAL(args.getNamedArgument("limit"), "10");
    BOOST_CHECK(args.hasArgument("verbose"));
    BOOST_CHECK(args.hasArgument("details"));

    args.clear();
    BOOST_CHECK_EQUAL(args.getTokenCount(), 0u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 0u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 0u);

    args.clear();
    args.parse("positional1 -limit 10 --verbose positional2");
    BOOST_CHECK_EQUAL(args.getTokenCount(), 5u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 2u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 2u);
    BOOST_CHECK_EQUAL(args.getNamedArgument("limit"), "10");
    BOOST_CHECK_EQUAL(args.getPositional(0), "positional1");
    BOOST_CHECK_EQUAL(args.getPositional(1), "positional2");

    args.clear();
    args.parse("positional1 -c --mobile");
    BOOST_CHECK_EQUAL(args.getTokenCount(), 3u);
    BOOST_CHECK_EQUAL(args.getPositionalCount(), 1u);
    BOOST_CHECK_EQUAL(args.getNamedCount(), 2u);
}

BOOST_AUTO_TEST_CASE(convertToBool)
{
    BOOST_CHECK_EQUAL(utils::convertToBool("true"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("TRUE"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("on"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("ON"), true);
    BOOST_CHECK_EQUAL(utils::convertToBool("1"), true);

    BOOST_CHECK_EQUAL(utils::convertToBool("false"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("FALSE"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("FalSe"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("off"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("OFF"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("OfF"), false);
    BOOST_CHECK_EQUAL(utils::convertToBool("0"), false);
    
    BOOST_CHECK_NE(utils::convertToBool("true"), false);
    BOOST_CHECK_NE(utils::convertToBool("false"), true);

    BOOST_CHECK_THROW(utils::convertToBool("asdsad"), std::runtime_error);
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

BOOST_AUTO_TEST_SUITE(Settings) // arccutils/Settings

void registerAllSettings(arcc::Settings& settings)
{
    using IntValidator = arcc::RangeValidator<std::int32_t>;
    using UIntValidator = arcc::RangeValidator<std::uint32_t>;

    settings.registerString("firstname", "value", std::make_shared<arcc::LengthValidator>(10));
    settings.registerString("lastname", "value", std::make_shared<arcc::LengthValidator>(10));
    settings.registerUInt("age", 30, std::make_shared<UIntValidator>(0, 150));
    settings.registerInt("int", -5, std::make_shared<IntValidator>(-10, 10));
    settings.registerBool("alive", true);
    settings.registerEnum("enum", "option1", { "option1", "option2", "option3" });
}


BOOST_AUTO_TEST_CASE(basicSettings) 
{
    arcc::Settings settings;
    registerAllSettings(settings);
   
    BOOST_CHECK_NO_THROW(settings.set("firstname", "bob"));
    BOOST_CHECK_NO_THROW(settings.set("lastname", "smith"));
    BOOST_CHECK_THROW(settings.set("firstname", "01234567890"), std::invalid_argument);

    BOOST_REQUIRE_NO_THROW(settings.set("age", "0"));
    BOOST_REQUIRE_NO_THROW(settings.set("age", "150"));
    BOOST_REQUIRE_THROW(settings.set("age", "250"), std::invalid_argument);
    BOOST_REQUIRE_THROW(settings.set("age", "-1"), std::invalid_argument);

    BOOST_REQUIRE_NO_THROW(settings.set("int", "-2"));
    BOOST_REQUIRE_NO_THROW(settings.set("int", "2"));
    BOOST_REQUIRE_THROW(settings.set("int", "-11"), std::invalid_argument);
    BOOST_REQUIRE_THROW(settings.set("int", "11"), std::invalid_argument);
    BOOST_TEST(!settings.setNoThrow("int", "123"));
    BOOST_TEST(!settings.setNoThrow("int", "test"));

    BOOST_REQUIRE_NO_THROW(settings.set("alive", "true"));
    BOOST_REQUIRE_NO_THROW(settings.set("alive", "off"));
    BOOST_REQUIRE_NO_THROW(settings.set("alive", "1"));
    BOOST_REQUIRE_THROW(settings.set("alive", "foo"), std::invalid_argument);

    BOOST_REQUIRE_NO_THROW(settings.set("enum", "option2"));
    BOOST_REQUIRE_THROW(settings.set("enum", "option4"), std::invalid_argument);
    BOOST_TEST(settings.setNoThrow("enum", "option2"));
    BOOST_TEST(!settings.setNoThrow("enum", "cat"));

    BOOST_TEST(settings.exists("firstname"));
    BOOST_TEST(settings.exists("lastname"));
    BOOST_TEST(!settings.exists("middlename"));
    
    BOOST_TEST(settings.value("firstname", "") == "bob"s);
    BOOST_TEST(settings.value("firstname", "") == "bob"s);
    BOOST_TEST(settings.value("age", 0u) == 150u);
    BOOST_TEST(settings.value("alive", false) == true);
    BOOST_TEST(settings.value("enum", "") == "option2"s);

    const auto temppath = boost::filesystem::temp_directory_path() 
        / boost::filesystem::unique_path("arcc%%%%");
    boost::filesystem::create_directories(temppath);

    const auto configfile = temppath / "settings.json";
    settings.save(configfile.string());

    arcc::Settings settings2;
    registerAllSettings(settings2);
    BOOST_TEST(settings.size() == settings2.load(configfile.string()));
    BOOST_TEST(settings2.value("firstname", ""s) == "bob"s);
    BOOST_TEST(settings2.value("age", 0u) == 150u);
    BOOST_TEST(settings2.value("alive", false) == true);
    BOOST_TEST(settings2.value("enum", ""s) == "option2"s);
}

BOOST_AUTO_TEST_SUITE_END() // arccutils/basicSettings

BOOST_AUTO_TEST_SUITE_END() // arccutils