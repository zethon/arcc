// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <json.hpp>
#include <rang.hpp>

#include "Terminal.h"
#include "utils.h"

#include "ConsoleApp.h"

namespace arcc
{
namespace console
{

ConsoleApp::ConsoleApp(Terminal& t)
    : _terminal(t), 
        _doExit(false), 
        _location("/")
{
}

void ConsoleApp::exec(const std::string& rawline)
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

void ConsoleApp::run()
{
    while (!_doExit)
    {
        printPrompt();
        std::string line = _terminal.getLine();
        exec(line);
    }
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint)
{
    return doRedditGet(endpoint, RedditSession::Params{});
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint, const RedditSession::Params& params)
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

std::string ConsoleApp::doSubRedditGet(const std::string& endpoint)
{
    return doRedditGet(endpoint, RedditSession::Params{});
}

std::string ConsoleApp::doSubRedditGet(const std::string& endpoint, const RedditSession::Params& params)
{
    std::string retval;

    if (_reddit)
    {
        retval = _reddit->doGetRequest(_location + endpoint, params);
    }
    else
    {
        std::cout << "you must be logged in first. type `login` to sign into reddit" << std::endl;
    }

    return retval;
}

bool ConsoleApp::loadSession()
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
            setRedditSession(std::make_shared<RedditSession>(
                j["accessToken"].get<std::string>(), 
                j["refreshToken"].get<std::string>(), 
                j["expiry"].get<double>(),
                j["time"].get<time_t>()));
        }
    }

    return _reddit != nullptr;
}

void ConsoleApp::saveSession()
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

void ConsoleApp::printPrompt() const
{
    std::cout 
        << (isLoggedIn() ? rang::fg::green : rang::fg::red)
        << '$'
        << _location
        << rang::fg::reset
        << "> ";

    std::cout << std::flush;
}  

} // namespace console
} // namespace arcc