// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

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

void initCommands()
{
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
        [](const std::string&)
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
