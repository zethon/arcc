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

void ConsoleApp::printError(const std::string& error)
{
    std::cout 
        << rang::fg::red
        << rang::style::bold
        << "error: " 
        << rang::fg::reset
        << rang::style::reset
        << error
        << std::endl;
}

void ConsoleApp::printStatus(const std::string& status)
{
    std::cout 
        << rang::fg::magenta
        << rang::style::bold
        << status
        << rang::fg::reset
        << rang::style::reset
        << std::endl;
}

ConsoleApp::ConsoleApp(Terminal& t)
    : _terminal(t), 
        _doExit(false), 
        _location("/")
{
}

ConsoleApp::~ConsoleApp()
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
                try
                {
                    // TODO: optimize out the std::string()
                    c.handler_(std::string(params));
                }
                catch (const std::exception& ex)
                {
                    std::cout << ex.what() << std::endl;
                }
                
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

    if (_reddit)
    {
        try
        {
            saveSession();
        }
        catch (const std::exception& ex)
        {
            printError(ex.what());
        }
    }
}

bool ConsoleApp::setLocation(const std::string& location)
{
    bool retval = false;

    if (location == "/")
    {
        _location.clear();
        retval = true;
    }
    else
    {
        const std::regex subRegex { R"(^\/r\/[a-zA-Z0-9]+$)" };

        if (std::regex_match(location, subRegex))
        {
            const std::string jstr = doSubRedditGet(location + "/about");
            if (jstr.size() > 0)
            {
                try
                {
                    auto jreply = nlohmann::json::parse(jstr);
                    if (jreply["data"]["created"].get<unsigned int>() > 0)
                    {
                        retval = true;
                        _location = location;
                    }
                }
                catch (const nlohmann::json::exception&)
                {
                }
            }
        }
    }
    
    return retval;
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
    return doSubRedditGet(endpoint, RedditSession::Params{});
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

        try
        {
            nlohmann::json j;
            i >> j;

            if (j.find("accessToken") != j.end() && j.find("refreshToken") != j.end() && j.find("expiry") != j.end())
            {
                setRedditSession(std::make_shared<RedditSession>(
                    j["accessToken"].get<std::string>(), 
                    j["refreshToken"].get<std::string>(), 
                    j["expiry"].get<double>(),
                    j["time"].get<time_t>()));

                    _location = j["location"].get<std::string>();
            }
        }
        catch (const nlohmann::json::exception& )
        {
            // TODO: should warn the user here
            resetSession();
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
    j["time"] = _reddit->lastRefresh();
    j["location"] = _location;

    std::ofstream out(sessionfile.string());
    out << j;
    out.close();
}

void ConsoleApp::resetSession()
{
    _reddit.reset(); 

    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_session";
    std::ofstream out(sessionfile.string());
    out << nlohmann::json{};
    out.close();
}

void ConsoleApp::printPrompt() const
{
    std::cout 
        << (isLoggedIn() ? rang::fg::green : rang::fg::red)
        << '$'
        << _location
        << rang::fg::gray
        << "> ";

    std::cout << std::flush;
}  

} // namespace console
} // namespace arcc