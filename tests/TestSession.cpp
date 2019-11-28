#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <nlohmann/json.hpp>

#include "../arcc/ConsoleApp.h"
#include "../arcc/RedditSession.h"

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(Session)

#ifndef _SESSION_FILE
#   error "_SESSION_FILE not defined, session unit tests cannot run"
#endif

arcc::RedditSessionPtr loadSession(const std::string& filename)
{
    if (boost::filesystem::exists(filename))
    {
        std::ifstream i(filename);
        nlohmann::json j;
        i >> j;

        return std::make_shared<arcc::RedditSession>(
            j["accessToken"].get<std::string>(),
            j["refreshToken"].get<std::string>(),
            j["expiry"].get<std::uint32_t>(),
            j["time"].get<time_t>());
    }

    return arcc::RedditSessionPtr{};
}

BOOST_AUTO_TEST_CASE(TestWhoAmI)
{
    auto sessionWeak = loadSession(_SESSION_FILE);
    auto session = sessionWeak.lock();
    BOOST_REQUIRE(session);

    const auto jsontext = session->doGetRequest("/api/v1/me");
    BOOST_REQUIRE(!jsontext.empty());

    nlohmann::json j = nlohmann::json::parse(jsontext);
    BOOST_CHECK(!j.value("name", "").empty());
}

BOOST_AUTO_TEST_CASE(testWeirdEndPoints)
{
    const std::vector<std::string> endpoints =
    {
        "/r/IAmA/top", "/r/IAmA/top/", "r/IAmA/top", "r/IAmA/top/"
    };

    auto sessionWeak = loadSession(_SESSION_FILE);
    auto session = sessionWeak.lock();
    BOOST_REQUIRE(session);

    for (const auto& endpoint : endpoints)
    {
        arcc::Params params{ {"t", "all"} };
        arcc::Listing listing{ session, endpoint , 2u, params };
        arcc::Listing::Page page = listing.getFirstPage();

        BOOST_REQUIRE(page.size() > 0);
        BOOST_REQUIRE_EQUAL(page.at(0).at("data").at("name"), "t3_z1c9z"s);
        BOOST_REQUIRE_EQUAL(page.at(1).at("data").at("name"), "t3_7eojwf"s);
    }
}

void testSubPages(const arcc::Listing::Page& otherpage, 
    const arcc::Listing::Page& basepage, 
    std::size_t baseOffset)
{
    std::size_t baseIdx = baseOffset;
    for (const auto& item : otherpage)
    {
        const auto& bigitem = basepage.at(baseIdx).at("data");
        BOOST_REQUIRE(item.find("data") != item.end());
        BOOST_REQUIRE(!item["data"].value("name", "").empty());
        BOOST_REQUIRE_EQUAL(item["data"]["name"], bigitem["name"]);
        baseIdx++;
    }
}

BOOST_AUTO_TEST_CASE(testNewSubListing)
{
    constexpr auto endpoint = "r/Omnism/new/";

    auto sessionWeak = loadSession(_SESSION_FILE);
    auto session = sessionWeak.lock();
    BOOST_REQUIRE(session);

    arcc::Listing biglist{ session, endpoint, 24u };
    arcc::Listing::Page bigpage = biglist.getFirstPage();
    BOOST_REQUIRE_EQUAL(bigpage.size(), 24u);
    BOOST_REQUIRE(biglist.before().empty());
    BOOST_REQUIRE(!biglist.after().empty());

    arcc::Listing listing{ session, endpoint, 4u };

    // test the first page
    arcc::Listing::Page page = listing.getFirstPage();
    testSubPages(page, bigpage, 0u);

    // test the second page
    page = listing.getNextPage();
    testSubPages(page, bigpage, 4u);

    // go back a page
    page = listing.getPreviousPage();
    testSubPages(page, bigpage, 0u);

    // go forward two pages
    listing.getNextPage();
    page = listing.getNextPage();
    testSubPages(page, bigpage, 8u);

    // go back two pages
    listing.getPreviousPage();
    page = listing.getPreviousPage();
    testSubPages(page, bigpage, 0u);

    // go fwd three pages
    listing.getNextPage();
    listing.getNextPage();
    page = listing.getNextPage();
    testSubPages(page, bigpage, 12u);
}

BOOST_AUTO_TEST_CASE(testHotSubListing)
{
    auto sessionWeak = loadSession(_SESSION_FILE);
    auto session = sessionWeak.lock();
    BOOST_REQUIRE(session);

    // Based off of this post: http://shorturl.at/btwH4, we know that
    // the all time top voted thread was Obama's AMA
    arcc::Params params{ {"t", "all"} };
    arcc::Listing listing{ session, "/r/IAmA/top" , 5u, params };
    arcc::Listing::Page page = listing.getFirstPage();

    BOOST_REQUIRE(page.size() > 0);
    BOOST_REQUIRE_EQUAL(page.at(0).at("data").at("name"), "t3_z1c9z"s);
    BOOST_REQUIRE_EQUAL(page.at(1).at("data").at("name"), "t3_7eojwf"s); 
}

BOOST_AUTO_TEST_SUITE_END() // Session


BOOST_AUTO_TEST_SUITE(GuestSession)

BOOST_AUTO_TEST_CASE(testGuestHotSubList)
{
    // create default guest session
    auto session = std::make_shared<arcc::RedditSession >(); 

    // Based off of this post: http://shorturl.at/btwH4, we know that
    // the all time top voted thread was Obama's AMA
    arcc::Params params{ {"t", "all"} };
    arcc::Listing listing{ session, "/r/IAmA/top" , 5u, params };
    arcc::Listing::Page page = listing.getFirstPage();

    BOOST_REQUIRE(page.size() > 0);
    BOOST_REQUIRE_EQUAL(page.at(0).at("data").at("name"), "t3_z1c9z"s);
    BOOST_REQUIRE_EQUAL(page.at(1).at("data").at("name"), "t3_7eojwf"s); 
}

BOOST_AUTO_TEST_SUITE_END() // GuestSession
