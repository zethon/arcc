// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>

#include <functional>
#include <string>
#include <vector>

#include <server_http.hpp>
#include <json.hpp>

#include "Terminal.h"
#include "WebClient.h"
#include "Reddit.h"

#define REDDIT_CLIENT_ID        "8T4M69w3Eop4YA"
#define REDDIT_RANDOM_STRING    "ArccClientForReddit"
#define REDDIT_REDIRECT_URL     "http://localhost:27182/oauth2"
#define REDDIT_SCOPE            "identity,edit,history,mysubreddits,privatemessages,read,save,submit,subscribe,vote"

namespace arcc
{

class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException()
    : std::logic_error("Function not yet implemented.")
    {
        // nothing to do
    }
};

namespace console
{

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

struct ConsoleCommand
{
    using Handler = std::function<void(const std::string&)>;

    std::vector<std::string>    commandNames_;
    std::string                 helpMessage_;
    Handler                     handler_;

    ConsoleCommand(const std::string& n, const std::string& hlp, Handler hdr)
        : helpMessage_(hlp), handler_(hdr)
    {
        boost::split(commandNames_, n, boost::is_any_of(","));
    }
};

using CommandHandler = ConsoleCommand::Handler;

class ConsoleApp final
{
    Terminal&                       _terminal;
    std::vector<ConsoleCommand>     _commands;
    bool                            _doExit;
    
    HttpServer                      _server;
    RedditSessionPtr                _reddit;
    
public:
    ConsoleApp(Terminal& t)
        : _terminal(t), _doExit(false)
    {
        _server.config.port = 27182;

        _server.resource["^/oauth2$"]["GET"] = 
            [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) 
            {
                // TODO: ERROR HANDLING, LOL

                std::stringstream stream;
                auto query_fields = request->parse_query_string();
                auto errorIt = query_fields.find("error");

                if (errorIt != query_fields.end())
                {
                    stream << "<h1>ERROR!</h1><h3>" << errorIt->second << "</h3>";
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
                    
                    auto jreply = nlohmann::json::parse(result.data);                    
                    _reddit = std::make_shared<RedditSession>(jreply["access_token"], jreply["refresh_token"], jreply["expires_in"].get<double>());

                    stream << "<h1>Login Successful!</h1>";
                    stream << "<p>You are now logged in to <b>Reddit</b> through <b>arcc</b> and may close this browser.";

                    response->write(stream);
                    response->send([&](const SimpleWeb::error_code & /*ec*/) 
                    { 
                        _server.stop(); // When send has finished, successful or not, stop the server
                    });

                    std::cout << "Login successful!" << std::endl;
                }
            };
    }

    ~ConsoleApp() = default;

    Terminal& getTerminal() { return _terminal; }

    HttpServer& getHttpServer() { return _server; }
    RedditSessionPtr getRedditSession() { return _reddit; }

    void addCommand(const ConsoleCommand& cmd)
    {
        _commands.push_back(cmd);
    }

    void doExitApp() { _doExit = true; }

    void exec(const std::string& rawline)
    {
        bool executed = false;

        std::cout << std::endl;

        std::string command { rawline };
        std::string params;

        boost::algorithm::trim(command);
        auto firstspace = command.find(' ');
        if (firstspace != std::string::npos)
        {
            params = command.substr(firstspace + 1);
            command = command.substr(0, firstspace);
        }

        for (auto& c : _commands)
        {
            for (auto& alias : c.commandNames_)
            {
                if (alias == command)
                {
                    // TODO: optimize out the std::string()
                    c.handler_(std::string(params));
                    executed = true;
                    goto endloop;
                }
            }
        }

        endloop:
        if (!executed)
        {
            std::cout << "Invalid command: " << command << std::endl;
        }
    }

    void run()
    {
        while (!_doExit)
        {
            std::cout << "> " << std::flush; // initial prompt
            std::string line = _terminal.getLine();
            exec(line);
        }
    }
};

} // namespace console
} // namespace arcc