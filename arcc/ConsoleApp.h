// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <nlohmann/json.hpp>

#include "Reddit.h"
#include "Terminal.h"
#include "CommandHistory.h"
#include "Listing.h"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::shared_ptr<RedditSession>;

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
    enum class ViewFormType { NORMAL, MOBILE, COMPACT, JSON };
    
    Terminal                        _terminal;
    RedditSessionPtr                _reddit;
    CommandHistory                  _history;

    std::vector<ConsoleCommand>     _commands;

    std::unique_ptr<Listing>        _listing;
    Listing::Page                   _currentPage;

    bool                            _doExit = false;
    std::string                     _location = "/";
    nlohmann::json                  _settings;

public:
    static void printError(const std::string& error);
    static void printWarning(const std::string& warning);
    static void printStatus(const std::string& status);
    
    ConsoleApp();

    std::string doRedditGet(const std::string& endpoint);
    std::string doRedditGet(const std::string& endpoint, const Params& params);

    // these will automatically prepred `_location` to the endpoint
    ListingPtr doGetListing(const arcc::Listing& listing);
    std::string doSubRedditGet(const std::string& endpoint, const Params& params);

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
    ViewFormType parserViewFormType(const std::string& val);

    void printPrompt() const;
    std::size_t printListing(const arcc::Listing& listing);

    void printListing();
    void renderLink(const nlohmann::json& link, std::size_t idx);

    void initCommands();
    void initTerminal();

    void initSettings();
    void saveSettings();
    void defaultSettings();
    void refreshSettings();

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
