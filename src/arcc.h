// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/algorithm/string.hpp>

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

    std::string doRedditGet(const std::string& endpoint, const RedditSession::Params& params = RedditSession::Params{})
    {
        std::string retval;

        if (_reddit)
        {
            retval = _reddit->doGetRequest(endpoint, params);
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
    void setRedditSession(RedditSessionPtr val) 
    { 
        _reddit = val; 
        saveSession();
    }

    bool loadSession()
    {
        boost::filesystem::path homefolder { utils::getUserFolder() };
        boost::filesystem::path sessionfile = homefolder / ".arcc_session";

        if (boost::filesystem::exists(sessionfile))
        {
            std::ifstream i(sessionfile.string());
            nlohmann::json j;
            i >> j;

            if (j.find("accessToken") != j.end() && j.find("refreshToken") != j.end() && j.find("expiry") != j.end())
            {
                _reddit = std::make_shared<RedditSession>(
                    j["accessToken"].get<std::string>(), 
                    j["refreshToken"].get<std::string>(), 
                    j["expiry"].get<double>(),
                    j["time"].get<time_t>());
            }
        }

        return _reddit != nullptr;
    }

    void saveSession()
    {
        boost::filesystem::path homefolder { utils::getUserFolder() };
        boost::filesystem::path sessionfile = homefolder / ".arcc_session";

        nlohmann::json j;

        j["accessToken"] = _reddit->accessToken();
        j["refreshToken"] = _reddit->refreshToken();
        j["expiry"] = _reddit->expiry();
        j["time"] = std::time(nullptr);

        std::ofstream out(sessionfile.string());
        out << j;
        out.close();
    }

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