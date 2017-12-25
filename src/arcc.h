// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>

#include <functional>
#include <string>
#include <vector>

#include <json.hpp>
#include <rang.hpp>

#include "Reddit.h"
#include "Terminal.h"

namespace arcc
{

namespace console
{

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
    ConsoleApp(Terminal& t)
        : _terminal(t), 
          _doExit(false), 
          _location("/")
    {
    }

    ~ConsoleApp() = default;

    Terminal& getTerminal() { return _terminal; }

    void addCommand(const std::string& n, const std::string& hlp, ConsoleCommand::Handler hdr)
    {
        _commands.push_back(ConsoleCommand{n,hlp,hdr});
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
            printPrompt();
            std::string line = _terminal.getLine();
            exec(line);
        }
    }

    std::string doReddit(const std::string& endpoint)
    {
        std::string retval;

        if (_reddit)
        {
            retval = _reddit->doRequest(endpoint);
        }
        else
        {
            std::cout << "you must be logged in first. type `login` to sign into reddit" << std::endl;
        }

        return retval;
    }

    void resetSession() { _reddit.reset(); }
    bool isLoggedIn() const { return _reddit != nullptr; }

    RedditSessionPtr getRedditSession() { return _reddit; }
    void setRedditSession(RedditSessionPtr val) { _reddit = val; }

private:
    void printPrompt() const
    {
        std::cout 
            << (isLoggedIn() ? rang::fg::green : rang::fg::red)
            << '$'
            << _location
            << rang::fg::reset
            << "> ";

        std::cout << std::flush;
    }    
};

} // namespace console
} // namespace arcc