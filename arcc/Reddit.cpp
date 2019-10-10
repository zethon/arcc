// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <ctime>

#include <nlohmann/json.hpp>

#include "core.h"
#include "utils.h"
#include "Reddit.h"

namespace arcc
{

void OAuth2Login::start()
{
    _server.resource["^/oauth2$"]["GET"] = 
        [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) 
        {
            std::string failedReason;

            std::stringstream stream;
            auto query_fields = request->parse_query_string();
            auto errorIt = query_fields.find("error");

            if (errorIt != query_fields.end())
            {
                failedReason = errorIt->second ;
            }
            else
            {
                auto codeIt = query_fields.find("code");

#ifndef NDEBUG
                auto stateIt = query_fields.find("state");
                assert(codeIt != query_fields.end() && stateIt != query_fields.end());
#endif

                WebClient client;
                client.setBasicAuth(REDDIT_CLIENT_ID,"");

                const std::string postData = (boost::format("grant_type=authorization_code&code=%1%&redirect_uri=%2%") % codeIt->second % REDDIT_REDIRECT_URL).str();
                auto result = client.doRequest("https://www.reddit.com/api/v1/access_token", postData, WebClient::Method::POST);
                
                if (result.status == 200)
                {
                    auto jreply = nlohmann::json::parse(result.data);
                    _reddit = std::make_shared<RedditSession>(jreply["access_token"], jreply["refresh_token"], jreply["expires_in"].get<double>());

                    _loggedIn = true;
                }
                else
                {
                    // TODO: need to support more failures if the retrieval to get the access token files
                    //       see https://github.com/reddit/reddit/wiki/oauth2#authorization
                    
                    if (result.status == 401)
                    {
                        failedReason = "http basic authorization credentials were invalid";
                    }
                    else
                    {
                        failedReason = "authorization failed for an unknown reason";
                    }
                }
            }

            if (_loggedIn)
            {
                stream << R"HTML(<!DOCTYPE html><html><head> <title>RTV OAuth2 Helper</title> <!-- style borrowed from http://bettermotherfuckingwebsite.com/ --> <style type="text/css"> body { margin:40px auto; max-width:650px; line-height:1.6; font-size:18px; font-family:Arial, Helvetica, sans-serif; color:#444; padding:0 10px; } h1, h2, h3 { line-height:1.2 } #footer { position: absolute; bottom: 0px; width: 100%; font-size:14px; } </style></head><body> <h1 style="color: green">Access Granted</h1><hr> <p><span style="font-weight: bold">Another Reddit Console Client</span> will now log in, you can close this window.</p> <div id="footer">View the <a href="https://github.com/zethon/arcc">Documentation</a></div></body></html>)HTML";
            }
            else
            {
                stream << R"HTML(<!DOCTYPE html><html><head> <title>RTV OAuth2 Helper</title> <!-- style borrowed from http://bettermotherfuckingwebsite.com/ --> <style type="text/css"> body { margin:40px auto; max-width:650px; line-height:1.6; font-size:18px; font-family:Arial, Helvetica, sans-serif; color:#444; padding:0 10px; } h1, h2, h3 { line-height:1.2 } #footer { position: absolute; bottom: 0px; width: 100%; font-size:14px; } </style></head><body> <h1 style="color: red">Access Denied</h1><hr> <p><span style="font-weight: bold">Another Reddit Console Client</span> could not log you in because:</p><pre>)HTML";
                stream << failedReason;
                stream << R"HTML(</pre><div id="footer">View the <a href="https://github.com/zethon/arcc">Documentation</a></div></body></html>)HTML";
            }

            response->write(stream);
            response->send([&](const SimpleWeb::error_code & /*ec*/) 
            { 
                _server.stop(); // When send has finished, successful or not, stop the server
            });
        };

    utils::openBrowser(_loginUrl);
    _server.start();
}

std::string buildQueryParamString(const RedditSession::Params& params)
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

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry)
    : RedditSession(accessToken, refreshToken, expiry, 0)
{
}

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry, time_t lastRefresh)
    : _accessToken(accessToken), 
        _refreshToken(refreshToken), 
        _expiry(expiry)
{
    const std::string userAgent = (boost::format("%1%:%2%:v%3% (by /u/wolosocu)") 
        % utils::getOsString() 
        % APP_TITLE 
        % VERSION).str();

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

std::string RedditSession::doGetRequest(const std::string& endpoint, const RedditSession::Params& params)
{
    doRefreshToken();

    std::string endpointUrl = requestUrl + endpoint + buildQueryParamString(params);
    auto result = _webclient.doRequest(endpointUrl);
    return result.data;
}

void RedditSession::doRefreshToken()
{
    std::time_t elapsed_seconds = std::time(0) - _lastRefresh;
    if (elapsed_seconds > _expiry)
    {
        WebClient client;
        client.setBasicAuth(REDDIT_CLIENT_ID,"");

        const std::string postData = (boost::format("grant_type=refresh_token&refresh_token=%1%") 
            % _refreshToken).str();

        auto result = client.doRequest("https://www.reddit.com/api/v1/access_token", postData, WebClient::Method::POST);

        if (result.status == 200)
        {
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

std::ostream& operator<<(std::ostream& os, const RedditSession::Params& params)
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
