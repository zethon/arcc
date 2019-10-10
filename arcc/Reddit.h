// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <chrono>
#include <memory>

#include <boost/format.hpp>
#include <server_http.hpp>

#include "Reddit.h"
#include "WebClient.h"

#define REDDIT_CLIENT_ID        "8T4M69w3Eop4YA"
#define REDDIT_RANDOM_STRING    "ArccClientForReddit"
#define REDDIT_REDIRECT_URL     "http://localhost:27182/oauth2"
#define REDDIT_SCOPE            "identity,edit,history,mysubreddits,privatemessages,read,save,submit,subscribe,vote"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::shared_ptr<RedditSession>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class OAuth2Login 
{
    const std::string               _loginUrl;
    HttpServer                      _server;
    bool                            _loggedIn = false;
    RedditSessionPtr                _reddit;

public:
    OAuth2Login()
        : _loginUrl((boost::format("https://ssl.reddit.com/api/v1/authorize?client_id=%1%&response_type=code&state=%2%&redirect_uri=%3%&duration=permanent&scope=%4%")
        % REDDIT_CLIENT_ID
        % REDDIT_RANDOM_STRING
        % REDDIT_REDIRECT_URL
        % REDDIT_SCOPE).str())
    {
        _server.config.port = 27182;
    }

    // TODO: this is a blocking call that will wait forever and returns once the user's 
    // browser loads and the user either clicks authorize or decline. Eventually there
    // should be a timeout OR a way in the terminal to allow the user to abort
    void start();

    bool loggedIn() { return _loggedIn; }
    RedditSessionPtr getRedditSession() { return _reddit; }
};

class RedditSession final
{
    std::function<void(void)>   _refreshCallback;

    const std::string           requestUrl = "https://oauth.reddit.com";

    std::string                 _accessToken;
    std::string                 _refreshToken;
    
    double                      _expiry;                // number of seconds until the session needs refresh
    time_t                      _lastRefresh;           // keep track so we know when to refresh our token

    WebClient                   _webclient;             // our "connection" to www.reddit.com

public:
    using Params = std::map<std::string, std::string>;

    RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry);
    RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry, time_t lastRefresh);

    std::string doGetRequest(const std::string& endpoint, const RedditSession::Params& params = RedditSession::Params{});

    std::string accessToken() const { return _accessToken; }
    std::string refreshToken() const { return _refreshToken; }
    double expiry() const { return _expiry; }
    time_t lastRefresh() const { return _lastRefresh; }

    void setRefreshCallback(std::function<void(void)> cb)
    {
        _refreshCallback = cb;
    }

private:
    void doRefreshToken();   
};

std::ostream & operator<<(std::ostream& os, const arcc::RedditSession::Params& params);

} // namespace arcc
