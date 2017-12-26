// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <ctime>

#include <json.hpp>

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
                auto stateIt = query_fields.find("state");

                assert(codeIt != query_fields.end() && stateIt != query_fields.end());

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
                stream << "<h1>Login Successful!</h1>";
                stream << "<p>You are now logged in to <b>Reddit</b> through <b>arcc</b> and may close this browser.";
            }
            else
            {
                stream << "<h1>ERROR!</h1>";
                stream << "<big>reason:</big> <b><font style='font-family:monospace;'>" << failedReason << "</font></b>";
                stream << "<p>You need to authorize <b>arcc</b> to use <b>Reddit</b>. You can try again by typing <b><i>login</i></b> in <b>arcc</b>";
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

RedditSession::RedditSession(const std::string& accessToken, const std::string& refreshToken, double expiry, time_t lastRefresh)
    : _accessToken(accessToken), 
        _refreshToken(refreshToken), 
        _expiry(expiry)
{
    const std::string userAgent = (boost::format("%1%:%2%:v%3% (by /u/wolosocu)") 
        % utils::getOsString() 
        % APP_NAME 
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
        }
    }
}


} // namespace arcc