// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <memory>

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

public:
    ConsoleApp(Terminal& t);
    ~ConsoleApp() = default;

    std::string doRedditGet(const std::string& endpoint);
    std::string doRedditGet(const std::string& endpoint, const RedditSession::Params& params);

    Terminal& getTerminal() { return _terminal; }

    void addCommand(const std::string& n, const std::string& hlp, ConsoleCommand::Handler hdr)
    {
        _commands.push_back(ConsoleCommand{n,hlp,hdr});
    }    

    void doExitApp() { _doExit = true; }

    void exec(const std::string& rawline);
    void run();

    void resetSession() { _reddit.reset(); }
    bool isLoggedIn() const { return _reddit != nullptr; }

    RedditSessionPtr getRedditSession() { return _reddit; }
    void setRedditSession(RedditSessionPtr val) 
    { 
        _reddit = val; 
        saveSession();
    }

    bool loadSession();
    void saveSession();

private:
    void printPrompt() const;
};

} // namespace console
} // namespace arcc