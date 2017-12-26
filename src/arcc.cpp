// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <cxxopts.hpp>

#include "core.h"
#include "Terminal.h"
#include "WebClient.h"
#include "utils.h"
#include "SimpleArgs.h"

#include "arcc.h"

namespace arcc
{

namespace console
{

// forward decl
void saveSession();

using ConsoleAppPtr = std::unique_ptr<ConsoleApp>;
ConsoleAppPtr consoleApp;

void printError(const std::string& error)
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

void whoami()
{
    auto jsontext = consoleApp->doRedditGet("/api/v1/me");

    if (jsontext.size() > 0)
    {
        auto jreply = nlohmann::json::parse(jsontext);

        std::cout << "username: " << jreply["name"].get<std::string>() << std::endl;

        std::time_t joined = jreply["created"].get<std::time_t>();
        std::cout << "joined  : " << std::asctime(std::localtime(&joined));
    }
}

void list(const std::string& cmdParams)
{
    static const std::vector<std::string> validTypes = {"new", "hot", "rising", "controversial", "top"};

    SimpleArgs args { cmdParams };
    args.parse();

    unsigned int limit = 5;
    std::string listType = "hot";

    if (args.getPositionalCount() > 0)
    {
        listType = args.getPositional(0);
        if (std::find(std::begin(validTypes), std::end(validTypes), listType) == std::end(validTypes))
        {
            printError("invalid list type '" + listType + "'");
            return;
        }
    }

    if (args.hasArgument("limit"))
    {
        auto limitStr = args.getNamedArgument("limit");
        try
        {
            limit = boost::lexical_cast<unsigned int>(limitStr);
        }
        catch (const boost::bad_lexical_cast&)
        {
            printError("parameter 'limit' has invalid value '" + limitStr + "'");
            return;
        }
    }

    RedditSession::Params params;
    params.insert(std::make_pair("limit", boost::lexical_cast<std::string>(limit)));

    auto jsontext = consoleApp->doRedditGet("/" + listType, params);

    unsigned int idx = 0;
    auto jreply = nlohmann::json::parse(jsontext);
    for (auto& child : jreply["data"]["children"])
    {
        std::cout 
            << rang::style::bold
            << ++idx
            << ". "
            << child["data"]["title"].get<std::string>()
            << rang::style::reset
            << '\n'
            << rang::fg::blue
            << rang::style::underline
            << child["data"]["url"].get<std::string>()
            << rang::style::reset
            << '\n'
            << rang::fg::reset
            << child["data"]["score"].get<int>() 
            << " pts • "
            << child["data"]["created"].get<int>() << " hr"
            << " - "
            << child["data"]["num_comments"].get<int>() << " comments"
            << '\n'
            << rang::fg::green
            << child["data"]["author"].get<std::string>()
            << ' '
            << rang::fg::yellow
            << child["data"]["subreddit_name_prefixed"].get<std::string>()
            << rang::fg::reset
            << std::endl;

            std::cout << std::endl;
    }
}

void initCommands()
{
    consoleApp->addCommand("list", "list stuff", std::bind(list, std::placeholders::_1));
    consoleApp->addCommand("whoami", "whoami", [](const std::string&) { whoami(); });
    consoleApp->addCommand("login", "login", [](const std::string& params)
        {
            if (!consoleApp->getRedditSession())
            {
                OAuth2Login login;
                login.start(); // will block until the login results are returned

                if (login.loggedIn())
                {
                    consoleApp->setRedditSession(login.getRedditSession());
                    saveSession();
                    std::cout << "login successful ヽ(´▽`)/" << std::endl;
                }
                else
                {
                    std::cout << "login denied (╯°□°）╯︵ ┻━┻" << std::endl;
                }
            }
            else
            {
                std::cout << "you are already logged in (/◔ ◡ ◔)/" << std::endl;
            }
        });

    consoleApp->addCommand("logout", "logout of the current session",[](const std::string&)
        {
            if (consoleApp->getRedditSession() != nullptr)
            {
                // write an empty session file
                boost::filesystem::path homefolder { utils::getUserFolder() };
                boost::filesystem::path sessionfile = homefolder / ".arcc_session";
                std::ofstream out(sessionfile.string());
                out << nlohmann::json{};
                out.close();                

                // delete our session object
                consoleApp->resetSession();

                std::cout << "you have logged out (•̀o•́)ง" << std::endl;
            }
            else
            {
                std::cout << "you are not logged in (•_•)" << std::endl;
            }
        });

    consoleApp->addCommand("q,quit,exit", "exit the program", 
        [](const std::string& params)
        {
            consoleApp->doExitApp();
        });

    consoleApp->addCommand("open,launch", "open a website in the default browser", 
        [](const std::string& params)
        {
            if (params.size() > 0)
            {
                std::vector<std::string> strings;
                boost::split(strings, params, boost::is_any_of(" "));
                
                std::string url = strings.at(0);

                // TODO: Apple's `CFURLRef` will want a properly formatted URL, so if the user enters
                // www.google.com, that will not be good enough. This is a naive solution to take 
                // what the user entered and turn it into a proper URL. Improve this.
                if (!boost::starts_with(url,"http://") && !boost::starts_with(url,"https://"))
                {
                    url = "http://" + url;
                }

                utils::openBrowser(url);
            }
            else
            {
                // show useage
                std::cout << "Usage: open <url>" << std::endl;
            }
        });

    consoleApp->addCommand("ping", "ping a website",
        [](const std::string& params)
        {
            try
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                WebClient client;
                auto result = client.doRequest(params);
                auto t_end = std::chrono::high_resolution_clock::now();
                std::cout << result.data.size() << " bytes in " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << " ms\n";
            }
            catch (WebClientError& e)
            {
                std::cout << "Could not ping because '" << e.what() << "'" << std::endl;
            }
        });
    consoleApp->addCommand("time", "ping a website",
        [](const std::string& params)
        {
            std::cout << std::time(nullptr) << std::endl;
        });
}

void saveSession()
{
    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_session";

    nlohmann::json j;

    auto reddit = consoleApp->getRedditSession();
    j["accessToken"] = reddit->accessToken();
    j["refreshToken"] = reddit->refreshToken();
    j["expiry"] = reddit->expiry();
    j["time"] = std::time(nullptr);

    std::ofstream out(sessionfile.string());
    out << j;
    out.close();
}

void loadSession()
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
            auto reddit = std::make_shared<RedditSession>(
                j["accessToken"].get<std::string>(), 
                j["refreshToken"].get<std::string>(), 
                j["expiry"].get<double>(),
                j["time"].get<time_t>());

            consoleApp->setRedditSession(reddit);

            std::cout << "loaded saved session" << std::endl;
        }
    }
}

} // namespace console
} // namespace arcc

int main(int argc, char* argv[])
{
    using namespace arcc;
    using namespace arcc::console;

    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    Terminal terminal;
    consoleApp = std::unique_ptr<ConsoleApp>(new ConsoleApp(terminal));

    initCommands();
    loadSession();

    try
    {
        consoleApp->run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "terminal error: " << ex.what() << std::endl;
    }

    return 0;
}