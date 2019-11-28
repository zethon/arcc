// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <memory>

#include <server_http.hpp>

#define REDDIT_CLIENT_ID        "8T4M69w3Eop4YA"
#define REDDIT_RANDOM_STRING    "ArccClientForReddit"
#define REDDIT_REDIRECT_URL     "http://localhost:27182/oauth2"
#define REDDIT_SCOPE            "identity,edit,history,mysubreddits,privatemessages,read,save,submit,subscribe,vote"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::weak_ptr<RedditSession>;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class OAuth2Login 
{
    const std::string               _loginUrl;
    HttpServer                      _server;
    bool                            _loggedIn = false;

    std::shared_ptr<arcc::RedditSession>    _reddit;

public:
    OAuth2Login();

    // TODO: this is a blocking call that will wait forever and returns once the user's 
    // browser loads and the user either clicks authorize or decline. Eventually there
    // should be a timeout OR a way in the terminal to allow the user to abort
    void start();

    bool loggedIn() { return _loggedIn; }

    std::shared_ptr<arcc::RedditSession>
        getRedditSession() { return _reddit; }
};

} // namespace
