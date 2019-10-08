// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <memory>

#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include "Reddit.h"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::shared_ptr<RedditSession>;

namespace console
{

class Terminal;

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
    RedditSessionPtr                _reddit;

    std::string                     _location;
    std::vector<nlohmann::json>     _lastObjects;

public:
    static void printError(const std::string& error);
    static void printStatus(const std::string& status);

    ConsoleApp(Terminal& t);
    ~ConsoleApp();

    std::string doRedditGet(const std::string& endpoint);
    std::string doRedditGet(const std::string& endpoint, const RedditSession::Params& params);

    // these will automatically prepred `_location` to the endpoint
    std::string doSubRedditGet(const std::string& endpoint);
    std::string doSubRedditGet(const std::string& endpoint, const RedditSession::Params& params);

    void exec(const std::string& rawline);
    void run();

    void addCommand(const std::string& n, const std::string& hlp, ConsoleCommand::Handler hdr)
    {
        _commands.push_back(ConsoleCommand{n,hlp,hdr});
    }    

    void doExitApp() { _doExit = true; }

    bool setLocation(const std::string&);
    bool isLoggedIn() const { return _reddit != nullptr; }

    RedditSessionPtr getRedditSession() { return _reddit; }
    void setRedditSession(RedditSessionPtr val) 
    { 
        _reddit = val;
        _reddit->setRefreshCallback(std::bind(&ConsoleApp::saveSession, this)); 
        saveSession();
    }

    bool loadSession();
    void saveSession();
    void resetSession();

    std::vector<nlohmann::json>& getLastObjects() { return _lastObjects; }

private:
    void whoami();
    void list(const std::string& params);
    void go(const std::string& params);

    void printPrompt() const;
};

} // namespace console
} // namespace arcc