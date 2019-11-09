// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <ctime>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>
#include <fmt/core.h>

#include "core.h"
#include "utils.h"
#include "OAuth2Login.h"

#include "Reddit.h"

namespace arcc
{

std::string buildQueryParamString(const Params& params)
{
    std::string retval;

    if (params.size() > 0)
    {
        retval.append("?");

        for (auto& param : params)
        {
            if (param.second.size() > 0)
            {
                retval += param.first + "=" + param.second + "&";
            }
        }
    }

    return retval;
}

RedditSession::RedditSession()
    : _expiry { 0 }
{
    const std::string userAgent = fmt::format("{}:{}:v{} (by /u/wolosocu)"
        ,utils::getOsString() 
        ,APP_TITLE 
        ,VERSION);

    _webclient.setUserAgent(userAgent);
    _webclient.setHeader("Authorization: bearer " + _accessToken);
}

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry)
    : RedditSession(accessToken, refreshToken, expiry, 0)
{
}

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry, time_t lastRefresh)
    : _accessToken(accessToken), 
        _refreshToken(refreshToken), 
        _expiry(expiry),
        _loggedIn(true)
{
    const std::string userAgent = fmt::format("{}:{}:v{} (by /u/wolosocu)"
        ,utils::getOsString() 
        ,APP_TITLE 
        ,VERSION);

    _webclient.setUserAgent(userAgent);
    _webclient.setHeader("Authorization: bearer " + _accessToken);

    if (lastRefresh != 0)
    {
        _lastRefresh = lastRefresh;
    }
    else
    {
        _lastRefresh = std::time(nullptr);
    }
}

std::string RedditSession::doGetRequest(
    const std::string& endpoint,
    const Params& params,
    bool verbose)
{
    doRefreshToken();

    // clean the endpoint since a malformed endpoint can
    // cause timeouts and other non-descript behavior
    std::string cleanpoint { endpoint };
    boost::algorithm::trim(cleanpoint);
    if (!cleanpoint.empty())
    {
        if (cleanpoint.at(0) != '/') cleanpoint.insert(0, "/");
        if (cleanpoint.at(cleanpoint.size()-1) == '/') cleanpoint.pop_back();
    }

    _lastRequest = oauthUrl + cleanpoint + buildQueryParamString(params);
    
    if (verbose)
    {
        std::cout << "request url: " << _lastRequest << std::endl;
    }

    auto result = _webclient.doRequest(_lastRequest);
    if (verbose)
    {
        std::cout << "response: " << result << std::endl;
    }

    return result.data;
}

void RedditSession::doRefreshToken()
{
    std::time_t elapsed_seconds = std::time(nullptr) - _lastRefresh;
    if (elapsed_seconds > _expiry)
    {
        WebClient client;
        std::string postData;

        if (loggedIn())
        {
            client.setBasicAuth(REDDIT_CLIENT_ID,"");
            postData = fmt::format("grant_type=refresh_token&refresh_token={}", _refreshToken);
        }
        else
        {
            client.setBasicAuth(REDDIT_CLIENT_ID,"client_secret??");
            postData = "grant_type=https://oauth.reddit.com/grants/installed_client&\\&device_id=34jr438r043j0438j043";
        }

        auto result = client.doRequest("https://www.reddit.com/api/v1/access_token", postData, WebClient::Method::POST);
        std::cout << "result: " << result.status << std::endl;

        if (result.status == 200)
        {
            std::cout << "data: " << result.data << std::endl;
            auto jreply = nlohmann::json::parse(result.data);


            _accessToken = jreply["access_token"].get<std::string>();
            _webclient.setHeader("Authorization: bearer " + _accessToken);

            _lastRefresh = std::time(nullptr);

            if (_refreshCallback)
            {
                _refreshCallback();
            }
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Params& params)
{
    char prefix = '{';

    for (const auto& [key, value] : params)
    {
        os << prefix
           << "{ "
           << key
           << " = "
           << value
           << " }";

        prefix = ',';
    }

    return os << '}';
}


} // namespace arcc
