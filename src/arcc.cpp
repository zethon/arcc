// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>


#include <cxxopts.hpp>

#include "core.h"
#include "Terminal.h"
#include "WebClient.h"

#include "arcc.h"

namespace arcc
{

namespace console
{

using ConsoleAppPtr = std::unique_ptr<ConsoleApp>;
ConsoleAppPtr consoleApp;

void initCommands()
{
    consoleApp->addCommand({"whoami", "whoami", [](const std::string& params)
    {
        consoleApp->doUserRequest("/api/v1/me");
    }});

    consoleApp->addCommand({"login", "login", [](const std::string& params)
    {
        OAuth2Login login;
        login.start(); // will block until the login results are returned

        if (login.loggedIn())
        {
            consoleApp->setRedditSession(login.getRedditSession());
            std::cout << "login successful c:" << std::endl;
        }
        else
        {
            std::cout << "login denied D:" << std::endl;
        }
    }});

    consoleApp->addCommand({"quit,exit", "exit the program", 
        [](const std::string& params)
        {
            consoleApp->doExitApp();
        }});

    consoleApp->addCommand({"open,launch", "open a website in the default browser", 
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
        }});

    consoleApp->addCommand({"ping", "ping a website",
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
        }});
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