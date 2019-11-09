// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <chrono>
#include <memory>

#include <boost/format.hpp>

#include "Listing.h"
#include "WebClient.h"

namespace arcc
{

class RedditSession final 
    : public std::enable_shared_from_this<RedditSession>
{
    std::function<void(void)>   _refreshCallback;

    const std::string           _oauthUrl = "https://oauth.reddit.com";

    std::string                 _accessToken;
    std::string                 _refreshToken;
    
    double                      _expiry;                // number of seconds until the session needs refresh
    time_t                      _lastRefresh;           // keep track so we know when to refresh our token
    WebClient                   _webclient;             // our "connection" to www.reddit.com

    bool                        _loggedIn = false;
    std::string                 _lastRequest;

public:

    RedditSession();
    RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry);
    RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry, time_t lastRefresh);

    std::string doGetRequest(const std::string& endpoint,
                              const Params& params = Params{},
                              bool verbose = false);

    std::string accessToken() const { return _accessToken; }
    std::string refreshToken() const { return _refreshToken; }
    double expiry() const { return _expiry; }
    time_t lastRefresh() const { return _lastRefresh; }

    bool loggedIn() const { return _loggedIn; }
    std::string lastRequest() const { return _lastRequest; }

    void setRefreshCallback(std::function<void(void)> cb)
    {
        _refreshCallback = cb;
    }

private:
    void doRefreshToken();   
};

std::ostream & operator<<(std::ostream& os, const arcc::Params& params);

} // namespace arcc
