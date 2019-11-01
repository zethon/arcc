// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include "Listing.h"

namespace arcc
{

Listing::Listing(SessionPtr session,
                 const std::string& endpoint,
                 std::size_t limit)
    : _sessionPtr{ session },
      _endpoint{ endpoint },
      _limit { limit }
{}

void Listing::initialize(nlohmann::json response)
{
    _response = std::move(response);

    const auto& data = _response.at("data");

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
}

Listing::Listing(const Listing& other)
    : _sessionPtr{ other._sessionPtr },
      _endpoint{ other.endpoint() },
      _limit { other.limit() }
{
}

ListingPtr Listing::getNextPage() const
{
    auto retval = std::make_unique<Listing>(*this);

    return retval;
}


ListingPtr Listing::getPreviousPage() const
{
    auto retval = std::make_unique<Listing>(*this);

    return retval;
}

//const nlohmann::json& Listing::data() const
//{
//    if (_results.find("data") == _results.end())
//    {
//        return std::nullopt;
//    }
//}

auto Listing::items() const -> const Listing::Items
{
    if (_response.find("data") == _response.end())
    {
        return std::nullopt;
    }

    if (_response["data"].find("children") == _response["data"].end())
    {
        return std::nullopt;
    }

    return _response["data"]["children"];
}

}
