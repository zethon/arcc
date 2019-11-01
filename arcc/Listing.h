// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

namespace arcc
{

class RedditSession;
using SessionPtr = std::weak_ptr<RedditSession>;

using Params = std::map<std::string, std::string>;

class Listing
{
    friend class RedditSession;

    SessionPtr                  _sessionPtr;

    std::string                 _endpoint;

    std::string                 _after;
    std::string                 _before;

    std::size_t                 _limit;
    std::size_t                 _count;

    nlohmann::json              _results;

public:

    Listing() = default;
    Listing(SessionPtr session, const std::string& endpoint);

    Listing getNextPage() const;
    Listing getPreviousPage() const;

    const nlohmann::json& results() const { return _results; }
    const nlohmann::json& children() const
    {
        return _results["data"]["children"];
    }

    std::string endpoint() const { return _endpoint; }
    std::string after() const { return _after; }
    std::string before() const { return _before; }
    std::size_t limit() const { return _limit; }
    std::size_t count() const { return _count; }
};

} // namespace