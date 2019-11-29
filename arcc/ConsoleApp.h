// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <nlohmann/json_fwd.hpp>

#include "AppBase.h"
#include "RedditSession.h"
#include "Terminal.h"
#include "CommandHistory.h"
#include "Listing.h"
#include "Settings.h"

namespace arcc
{

class RedditSession;
using RedditSessionPtr = std::weak_ptr<RedditSession>;

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

class ConsoleApp final : public AppBase
{
    enum class ViewFormType { NORMAL, MOBILE, COMPACT, JSON };
    
    Terminal                        _terminal;
    CommandHistory                  _history;

    std::vector<ConsoleCommand>     _commands;

    std::unique_ptr<Listing>        _listing;
    Listing::Page                   _currentPage;

    bool                            _doExit = false;

    arcc::Settings&                         _settings;
    std::shared_ptr<arcc::RedditSession>    _session;

public:
    static void printError(const std::string& error);
    static void printWarning(const std::string& warning);
    static void printStatus(const std::string& status);
    
    ConsoleApp(arcc::Settings& settings, std::shared_ptr<arcc::RedditSession> session);

    std::string doRedditGet(const std::string& endpoint);
    std::string doRedditGet(const std::string& endpoint, const Params& params);

    void exec(const std::string& rawline);
    void run() override;

    void addCommand(const std::string& n, const std::string& hlp, ConsoleCommand::Handler hdr)
    {
        _commands.push_back(ConsoleCommand{n,hlp,hdr});
    }    

    void doExitApp() { _doExit = true; }

    

private:
    void openIndex(std::size_t index);

    ViewFormType parserViewFormType(const std::string& val);
    
    bool setLocation(const std::string&);

    void printPrompt() const;

    void printListing();
    void renderLink(const nlohmann::json& link, std::size_t idx);

    void initCommands();
    void initTerminal();

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
