// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>

#include <nlohmann/json.hpp>

#include <rang.hpp>

#include "core.h"
#include "Terminal.h"
#include "WebClient.h"
#include "utils.h"
#include "SimpleArgs.h"

#include "ConsoleApp.h"

namespace arcc
{

namespace console
{

// forward decl
void saveSession();

using ConsoleAppPtr = std::unique_ptr<ConsoleApp>;
ConsoleAppPtr consoleApp;

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

    unsigned int limit = 5;
    std::string listType = "hot";
    std::string subReddit;

    if (args.getPositionalCount() > 0)
    {
        listType = args.getPositional(0);
        if (std::find(std::begin(validTypes), std::end(validTypes), listType) == std::end(validTypes))
        {
            ConsoleApp::printError("invalid list type '" + listType + "'");
            return;
        }
    }

    if (args.hasArgument("sub"))
    {
        subReddit = args.getNamedArgument("sub");
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
            ConsoleApp::printError("parameter 'limit' has invalid value '" + limitStr + "'");
            return;
        }
    }

    RedditSession::Params params;
    params.insert(std::make_pair("limit", boost::lexical_cast<std::string>(limit)));

    auto jsontext = consoleApp->doSubRedditGet(subReddit + "/" + listType, params);
    if (jsontext.size() > 0)
    {
        const auto jreply = nlohmann::json::parse(jsontext);

        if (args.hasArgument("json"))
        {
            // roundtrip the JSON so we can get pretty indentation
            std::cout << jreply.dump(4) << std::endl;
        }
        else
        {
            unsigned int idx = 0;

            auto& lastObjects = consoleApp->getLastObjects();
            lastObjects.clear();
            
            for (const auto& child : jreply["data"]["children"])
            {
                std::string flairText;
                try
                {
                    flairText = child["data"]["link_flair_text"].get<std::string>();
                }
                catch (const nlohmann::json::type_error&)
                {
                    // swallow this
                }

                if (flairText.size() > 0)
                {
                    flairText = "[" + flairText + "]";
                }

                if (child["data"]["stickied"].get<bool>())
                {
                    std::cout << rang::fg::black << rang::style::bold << rang::bg::yellow;
                }                

                std::cout 
                    << rang::style::bold
                    << ++idx
                    << ". "
                    << child["data"]["title"].get<std::string>()
                    << rang::style::reset
                    << '\n'
                    << rang::fg::cyan
                    << rang::style::underline
                    << child["data"]["url"].get<std::string>()
                    << rang::style::reset
                    << '\n'
                    << rang::fg::gray
                    << child["data"]["score"].get<int>() 
                    << " pts - "
                    << utils::miniMoment(child["data"]["created_utc"].get<int>()) 
                    << " - "
                    << child["data"]["num_comments"].get<int>() << " comments"
                    << '\n'
                    << rang::fg::magenta
                    << child["data"]["author"].get<std::string>()
                    << ' '
                    << rang::fg::yellow
                    << child["data"]["subreddit_name_prefixed"].get<std::string>();

                if (flairText.size() > 0)
                {
                    std::cout
                        << ' '
                        << rang::fg::red
                        << flairText;
                }

                std::cout
                    << rang::fg::reset
                    << rang::bg::reset
                    << rang::style::reset
                    << '\n'
                    << std::endl;

                lastObjects.push_back(child);
            }
        }
    }
}

void go(const std::string& params)
{
    SimpleArgs args { params };
    if (args.getTokenCount() > 0)
    {
        auto& location = args.getToken(0);
        if (!boost::istarts_with(location, "/r/"))
        {
            location = "/r/" + location;
        }

        if (!consoleApp->setLocation(location))
        {
            ConsoleApp::printError("invalid subreddit '" + args.getToken(0) + "'");
        }
    }
    else
    {
       ConsoleApp::printError("no subreddit specified");
    }
}

void initCommands()
{
    consoleApp->addCommand("go", "go to a subreddit", std::bind(go, std::placeholders::_1));
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
                    std::cout << "login successful " << utils::sentimentText(utils::Sentiment::POSITIVE) << std::endl;
                }
                else
                {
                    std::cout << "login denied " << utils::sentimentText(utils::Sentiment::NEGATIVE) << std::endl;
                }
            }
            else
            {
                std::cout << "you are already logged in " << utils::sentimentText(utils::Sentiment::POSITIVE) << std::endl;
            }
        });

    consoleApp->addCommand("logout", "logout of the current session",[](const std::string&)
        {
            if (consoleApp->getRedditSession() != nullptr)
            {
                // set location to the root
                consoleApp->setLocation("/");

                // delete our session object
                consoleApp->resetSession();

                std::cout << "you have logged out " << utils::sentimentText(utils::Sentiment::NEGATIVE) << std::endl;
            }
            else
            {
                std::cout << "you are not logged in " << utils::sentimentText(utils::Sentiment::NEUTRAL) << std::endl;
            }
        });

    consoleApp->addCommand("q,quit,exit", "exit the program", 
        [](const std::string& params)
        {
            consoleApp->doExitApp();
        });

    consoleApp->addCommand("v,view", "view a listed item's link or comments",
        [](const std::string& params)
        {
            SimpleArgs args{ params };
            if (args.getPositionalCount() > 0)
            {
                std::string url;

                if (const auto & firstarg = args.getPositional(0); utils::isNumeric(firstarg))
                {
                    auto index = std::stoul(firstarg);
                    const auto& lastObjects = consoleApp->getLastObjects();

                    if (index > 0 && index <= lastObjects.size())
                    {
                        const auto& object = lastObjects.at(index - 1);

                        if (args.hasArgument("comments") || args.hasArgument("c"))
                        {
                            url = "https://www.reddit.com" + object["data"].value("permalink", "");
                        }
                        else if (args.hasArgument("url") || args.hasArgument("u"))
                        {
                            url = object["data"].value("url", "");
                        }
                        else
                        {
                            ConsoleApp::printError("usage: view <index> (-c,--comments | -u,--url)");
                        }

                        utils::openBrowser(url);
                    }
                    else
                    {
                        ConsoleApp::printError("index out of range!");
                    }
                }
                else
                {
                    ConsoleApp::printError("usage: view <index> (-c,--comments | -u,--url)");
                }
            }
            else
            {
                ConsoleApp::printError("usage: view <index> (-c,--comments | -u,--url)");
            }
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

    consoleApp->addCommand("about", "information about this arcc",
        [](const std::string&) { utils::openBrowser("https://github.com/zethon/arcc"); });
}

} // namespace console
} // namespace arcc

int main(int, char*[])
{
    using namespace arcc;
    using namespace arcc::console;

    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    Terminal terminal;
    consoleApp = std::unique_ptr<ConsoleApp>(new ConsoleApp(terminal));

    initCommands();

    if (consoleApp->loadSession())
    {
        ConsoleApp::printStatus("saved session restored");
    }

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
