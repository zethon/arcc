// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include "Reddit.h"
#include "Terminal.h"
#include "CommandHistory.h"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::shared_ptr<RedditSession>;

struct Listing
{
    std::string                 subreddit;
    std::string                 type;

    std::string                 before;
    std::string                 after;

    std::uint32_t               count;
    std::uint32_t               limit;

    RedditSession::Params       params;
    nlohmann::json              results;
    bool                        verbose;

    std::string endpoint() const
    {
        std::string retval;
        if (!subreddit.empty() && subreddit != "/")
        {
            if (!boost::starts_with(subreddit, "/r"))
            {
                retval.append("/r");
            }

            retval.append(subreddit);
        }

        // some times a "/" can sneak into the subreddit
        // name so check for it before creating a "//"
        // which can be problematic
        if (!boost::ends_with(retval, "/")) retval.append("/");

        // now at `hot`, `new`, etc...
        retval.append(type);
        return retval;
    }

    void reset()
    {
        subreddit.clear();
        type.clear();
        params.clear();
        before.clear();
        after.clear();
        results.clear();
        verbose = false;
        count = 0;
        limit = 0;
    }
};

struct ConsoleCommand
{
    using Handler = std::function<void(const std::string&)>;

    std::vector<std::string>    commandNames_;
    std::string                 helpMessage_;
    std::string                 usage_;
    Handler                     handler_;

    ConsoleCommand(const std::string& n, const std::string& hlp, Handler hdr)
        : ConsoleCommand(n, hlp, std::string{}, hdr) {}

    ConsoleCommand(const std::string& n, const std::string& hlp, const std::string& usage, Handler hdr);
};

using CommandHandler = ConsoleCommand::Handler;

class ConsoleApp final
{
    Terminal                        _terminal;
    RedditSessionPtr                _reddit;
    CommandHistory                  _history;

    std::vector<ConsoleCommand>     _commands;

    Listing                         _listing;
    std::vector<nlohmann::json>     _lastObjects;

    bool                            _doExit = false;
    std::string                     _location = "/";
    nlohmann::json                  _settings;

public:
    static void printError(const std::string& error);
    static void printStatus(const std::string& status);

    ConsoleApp();

    std::string doRedditGet(const std::string& endpoint);
    std::string doRedditGet(const std::string& endpoint, const RedditSession::Params& params);

    // these will automatically prepred `_location` to the endpoint
    arcc::Listing doGetListing(const arcc::Listing& listing);
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

private:
    void printPrompt() const;
    void printListing(const arcc::Listing& listing);

    void initCommands();
    void initTerminal();

    void initSettings();
    void saveSettings();
    void defaultSettings();

    void whoami();
    void list(const std::string& params);
    void go(const std::string& params);
    void view(const std::string& params);
    void help(const std::string& params);
    void history(const std::string& params);
    void next(const std::string& params);
    void previous(const std::string& params);

    void setCommand(const std::string& params);
    void settingsCommand(const std::string& params);
};

} // namespace arcc
