// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <ctime>

#include <json.hpp>

#include "Reddit.h"

const std::string DEFAULT_USERAGENT = "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X x.y; rv:42.0) Gecko/20100101 Firefox/42.0";

namespace arcc
{

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry)
    : _accessToken(accessToken), 
        _refreshToken(refreshToken), 
        _expiry(expiry),
        _lastRefresh(std::chrono::system_clock::now())
{
    _webclient.setUserAgent(DEFAULT_USERAGENT);
    _webclient.setHeader("Authorization: bearer " + _accessToken);
}

std::string RedditSession::doRequest(const std::string& endpoint, WebClient::Method method)
{
    refreshToken();

    std::string endpointUrl = requestUrl + endpoint;

    auto result = _webclient.doRequest(endpointUrl, std::string(), method);
    auto jreply = nlohmann::json::parse(result.data);

    std::cout << "username: " << jreply["name"].get<std::string>() << std::endl;
    
    std::time_t joined = jreply["created"].get<std::time_t>();
    std::cout << "joined  : " << std::asctime(std::localtime(&joined));

    return result.data;
}


} // namespace arcc