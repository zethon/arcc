#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include "../arcc/ConsoleApp.h"
#include "../arcc/Reddit.h"

BOOST_AUTO_TEST_SUITE(Session)

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
    auto session = loadSession(R"(C:\Users\aclau\.arcc_session)");

    const auto& [jsontext, url] = session->doGetRequest("/api/v1/me");
    BOOST_REQUIRE(!jsontext.empty());
    BOOST_CHECK(!url.empty());

    nlohmann::json j = nlohmann::json::parse(jsontext);
    BOOST_CHECK(!j.value("name", "").empty());
}

BOOST_AUTO_TEST_CASE(TestListing)
{
    // https://www.reddit.com/r/deadsubs/new/ is a pretty dead sub so the goal
    // here is to get the first 20 threads, and then paginate through them

    auto session = loadSession(R"(C:\Users\aclau\.arcc_session)");

    arcc::Params params;
    params.insert_or_assign("count", "20");
    auto listing = session->getListing("/r/deadsubs/new/", params);
    //listing = listing.getNextPage();
    //listing = listing.getPreviousPage();
}

BOOST_AUTO_TEST_SUITE_END()