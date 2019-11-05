// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>


#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "Reddit.h"
#include "WebClient.h"
#include "utils.h"

#include "OAuth2Login.h"

namespace arcc
{

OAuth2Login::OAuth2Login()
    : _loginUrl{fmt::format("https://ssl.reddit.com/api/v1/authorize?client_id={}&response_type=code&state={}&redirect_uri={}&duration=permanent&scope={}",
    REDDIT_CLIENT_ID, 
    REDDIT_RANDOM_STRING, 
    REDDIT_REDIRECT_URL, 
    REDDIT_SCOPE)}
{
    _server.config.port = 27182;
}

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

                const std::string postData = 
                    fmt::format("grant_type=authorization_code&code={}&redirect_uri={}", 
                        codeIt->second, REDDIT_REDIRECT_URL);

                auto result = client.doRequest("https://www.reddit.com/api/v1/access_token", 
                    postData, WebClient::Method::POST);
                
                if (result.status == 200)
                {
                    auto jreply = nlohmann::json::parse(result.data);
                    _reddit = std::make_shared<RedditSession>(jreply["access_token"], jreply["refresh_token"], jreply["expires_in"].get<double>());

                    _loggedIn = true;
                }
                else
                {
                    // TODO: need to support more failures if the retrieval to get the access token fails
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


} // namespace