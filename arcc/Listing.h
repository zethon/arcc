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

    std::string                 _after;
    std::string                 _before;

    std::size_t                 _limit;
    std::size_t                 _count;

    std::string                 _endpoint;

    nlohmann::json              _results;

    std::string                 subreddit;
    std::string                 type;

    Params                      params;
    bool                        details;
    bool                        verbose;

public:

    void setSession(SessionPtr val) { _sessionPtr = val; }

    void setEndpoint(const std::string& val) { _endpoint = val; }
    std::string endpoint() const { return _endpoint; }

    std::string after() const { return _after; }
    std::string before() const { return _before; }

    std::size_t limit() const { return _limit; }
    std::size_t count() const { return _count; }

    const nlohmann::json& results() const { return _results; }

    void reset();
};

} // namespace