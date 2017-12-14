// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>

#include <functional>
#include <string>
#include <vector>

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
    std::vector<ConsoleCommand>     _commands;
    std::function<void(void)>       _exitHandler;

public:
    ConsoleApp() = default;
    ~ConsoleApp() = default;

    void addCommand(const ConsoleCommand& cmd)
    {
        _commands.push_back(cmd);
    }

    void setExitHandler(std::function<void(void)> handler)
    {
        _exitHandler = handler;
    }

    void invokeExitHandler()
    {
        if (_exitHandler)
        {
            _exitHandler();
        }
    }

    void exec(const std::string& command, const std::string& params)
    {
        bool executed = false;

        for (auto& c : _commands)
        {
            for (auto& alias : c.commandNames_)
            {
                if (alias == command)
                {
                    c.handler_(params);
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
};

} // namespace console
} // namespace arcc