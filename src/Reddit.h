// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <chrono>
#include <memory>

#include "WebClient.h"

namespace arcc
{

class RedditSession final
{
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    const std::string requestUrl = "https://oauth.reddit.com";

    std::string     _accessToken;
    std::string     _refreshToken;
    
    double          _expiry;        // number of seconds until the session needs refresh
    TimePoint       _lastRefresh;

    WebClient       _webclient;

public:
    RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry);

    std::string doRequest(const std::string& endpoint, WebClient::Method method = WebClient::Method::GET);

private:
    void refreshToken()
    {
        std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now()-_lastRefresh;
        if (elapsed_seconds.count() > _expiry)
        {
            // send refresh request

            _lastRefresh = std::chrono::system_clock::now();
        }
    }    
};

using RedditSessionPtr = std::shared_ptr<RedditSession>;

} // namespace arcc