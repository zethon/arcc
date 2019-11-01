#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <nlohmann/json.hpp>

#include "../arcc/ConsoleApp.h"
#include "../arcc/Reddit.h"

using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(Session)

#ifdef _WINDOWS
constexpr auto SESSION_FILE = R"(C:\Users\aclau\.arcc_session)";
#else
constexpr auto SESSION_FILE = R"(/Users/adalidclaure/.arcc_session)";
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
            j["expiry"].get<double>(),
            j["time"].get<time_t>());
    }

    return nullptr;
}

BOOST_AUTO_TEST_CASE(TestWhoAmI)
{
    auto session = loadSession(SESSION_FILE);

    const auto& [jsontext, url] = session->doGetRequest("/api/v1/me");
    BOOST_REQUIRE(!jsontext.empty());
    BOOST_CHECK(!url.empty());

    nlohmann::json j = nlohmann::json::parse(jsontext);
    BOOST_CHECK(!j.value("name", "").empty());
}

BOOST_AUTO_TEST_CASE(testBadListings)
{
    auto session = loadSession(SESSION_FILE);
    arcc::Listing listing = session->getListing("/r/lsdjkhfdsjh0309902");
    BOOST_REQUIRE_EQUAL(listing.items()->get().size(), 0u);
}

BOOST_AUTO_TEST_CASE(testSubListing)
{
    // https://www.reddit.com/r/deadsubs/new/ is a pretty dead sub so the goal
    // here is to get the first 20 threads, and then paginate through them

    auto session = loadSession(SESSION_FILE);

    arcc::Params params;
    params.insert_or_assign("limit", "2");
    auto biglist = session->getListing("/r/deadsubs/new/", params, 24);
    BOOST_REQUIRE_EQUAL(biglist.endpoint(), "/r/deadsubs/new/"s);
    BOOST_REQUIRE(biglist.items());
    BOOST_REQUIRE_EQUAL(biglist.items()->get().size(), 24u);
    BOOST_REQUIRE(biglist.before().empty());
    BOOST_REQUIRE(!biglist.after().empty());

    params.clear();
    params.insert_or_assign("limit", "4");
    auto listing = session->getListing("/r/deadsubs/new", params);
    BOOST_REQUIRE_EQUAL(listing.items()->get().size(), 4u);
    BOOST_CHECK(listing.before().empty());
    BOOST_CHECK_EQUAL(listing.after(), biglist.items()->get().at(3).at("data").at("name"));

    for (auto idx = 0u; idx < 4; idx++)
    {
        const auto& bigitem  = biglist.items()->get().at(idx).at("data");
        const auto& item = listing.items()->get().at(idx).at("data");
        BOOST_CHECK_EQUAL(item.at("name"), bigitem.at("name"));
    }

    listing = listing.getNextPage();
    BOOST_REQUIRE_EQUAL(listing.items()->get().size(), 4u);
    for (auto idx = 0u; idx < 4; idx++)
    {
        const auto& bigitem  = biglist.items()->get().at(idx+4).at("data");
        const auto& item = listing.items()->get().at(idx).at("data");
        BOOST_CHECK_EQUAL(item.at("name"), bigitem.at("name"));
    }

    //listing = listing.getPreviousPage();
}

BOOST_AUTO_TEST_SUITE_END()
