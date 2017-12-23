// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>

#include <functional>
#include <string>
#include <vector>

#include <json.hpp>

#include "Reddit.h"
#include "Terminal.h"

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

public:
    ConsoleApp(Terminal& t)
        : _terminal(t), _doExit(false)
    {
    }

    ~ConsoleApp() = default;

    Terminal& getTerminal() { return _terminal; }

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

    void doUserRequest(const std::string& endpoint)
    {
        if (_reddit)
        {
            _reddit->doRequest(endpoint);
        }
        else
        {
            std::cout << "you must be logged in first! type `login` to sign into reddit" << std::endl;
        }
    }

    RedditSessionPtr getRedditSession() { return _reddit; }
    void setRedditSession(RedditSessionPtr val) { _reddit = val; }
};

} // namespace console
} // namespace arcc