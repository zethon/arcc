// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include "RedditSession.h"
#include "Listing.h"

namespace arcc
{

Listing::Listing(RedditSessionPtr session,
                 const std::string& endpoint,
                 std::size_t limit)
    : Listing(session, endpoint, limit, Params{})
{}

Listing::Listing(RedditSessionPtr session, const std::string& endpoint, std::size_t limit, const Params& params)
    : _sessionPtr{ session },
    _endpoint{ endpoint },
    _limit{ limit },
    _params{ params }
{}

Listing::Listing(const Listing& other)
    : _sessionPtr{ other._sessionPtr },
    _endpoint{ other.endpoint() },
    _limit{ other.limit() },
    _params{ other.params() }
{
}

Listing::Page Listing::processResponse(nlohmann::json response)
{
    Listing::Page retval;

    const auto& data = response.at("data");

    if (data.find("before") != data.end()
        && !data["before"].is_null())
    {
        _before = data["before"];
    }

    if (data.find("after") != data.end()
        && !data["after"].is_null())
    {
        _after = data["after"];
    }

    return data.value("children", Page{});
}

Listing::Page Listing::getFirstPage()
{
    if (auto session = _sessionPtr.lock(); session)
    {
        Params params{ _params };
        params.insert_or_assign("limit", std::to_string(_limit));

        const auto jsontext = session->doGetRequest(_endpoint, params);
        if (!jsontext.empty())
        {
            const auto reply = nlohmann::json::parse(jsontext);
            if (reply.find("data") == reply.end())
            {
                throw std::runtime_error("the listing response was malformed");
            }

            return processResponse(reply);
        }
    }
    
    return Listing::Page{};
}

Listing::Page Listing::getNextPage()
{
    if (_after.empty())
    {
        // no more posts
        return Listing::Page{};
    }

    if (auto session = _sessionPtr.lock(); session)
    {
        _before.clear();
        _count += _limit;

        Params params{ _params };
        params.insert_or_assign("limit", std::to_string(_limit));
        params.insert_or_assign("count", std::to_string(_count));
        params.insert_or_assign("after", _after);

        const auto jsontext = session->doGetRequest(_endpoint, params);
        if (!jsontext.empty())
        {
            const auto reply = nlohmann::json::parse(jsontext);
            if (reply.find("data") == reply.end())
            {
                throw std::runtime_error("the listing response was malformed");
            }

            return processResponse(reply);
        }
    }

    return Listing::Page{};
}


Listing::Page Listing::getPreviousPage()
{
    if (_before.empty()
        || (_count - _limit) <= 0)
    {
        // we're at the first page, so no more pages
        return Listing::Page{};
    }

    if (auto session = _sessionPtr.lock(); session)
    {
        _after.clear();
        _count -= (_limit - 1);

        Params params{ _params };
        params.insert_or_assign("limit", std::to_string(_limit));
        params.insert_or_assign("before", _before);

        if (_count > 0)
        {
            params.insert_or_assign("count", std::to_string(_count));
        }

        const auto jsontext = session->doGetRequest(_endpoint, params);
        if (!jsontext.empty())
        {
            const auto reply = nlohmann::json::parse(jsontext);
            if (reply.find("data") == reply.end())
            {
                throw std::runtime_error("the listing response was malformed");
            }

            return processResponse(reply);
        }
    }

    return Listing::Page{};
}

}
