// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/format.hpp>

#ifdef _WINDOWS
#include <windows.h>
#include <shellapi.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <cxxopts.hpp>

#include "core.h"
#include "Terminal.h"
#include "WebClient.h"

#include "arcc.h"

namespace arcc
{

namespace console
{

void openBrowser(const std::string& url_str)
{
#ifdef _WINDOWS
   ShellExecute(0, 0, url_str.c_str(), 0, 0, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    // only works with `http://` prepended
    CFURLRef url = CFURLCreateWithBytes (
        NULL,                        // allocator
        (UInt8*)url_str.c_str(),     // URLBytes
        url_str.length(),            // length
        kCFStringEncodingASCII,      // encoding
        NULL                         // baseURL
    );
    
    LSOpenCFURLRef(url,0);
    CFRelease(url);
#else
    throw NotImplementedException();
#endif
}

using ConsoleAppPtr = std::unique_ptr<ConsoleApp>;
ConsoleAppPtr consoleApp;

void login()
{
    // auto& terminal = consoleApp->getTerminal();

    const std::string oauth2url = 
        (boost::format("https://ssl.reddit.com/api/v1/authorize?client_id=%1%&response_type=code&state=%2%&redirect_uri=%3%&duration=permanent&scope=%4%")
        % REDDIT_CLIENT_ID
        % REDDIT_RANDOM_STRING
        % REDDIT_REDIRECT_URL
        % REDDIT_SCOPE).str();

    openBrowser(oauth2url);
    consoleApp->getHttpServer().start();
}

void initCommands()
{
    consoleApp->addCommand({"whoami", "whoami", [](const std::string& params)
    {
        auto reddit = consoleApp->getRedditSession();
        reddit->doRequest("/api/v1/me");
    }});

    consoleApp->addCommand({"login", "login", [](const std::string& params)
    {
        login();
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

                openBrowser(url);
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