// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include <boost/filesystem.hpp>

#include "core.h"
#include "ConsoleApp.h"
#include "Settings.h"

arcc::Settings registerAllSettings()
{
    arcc::Settings settings;

    settings.registerBool("global.terminal.color", true);
    settings.registerBool("command.go.autolist", true);
    settings.registerUInt("command.list.limit", 5);
    settings.registerEnum("command.list.type", "hot", { "new", "hot", "rising", "controversial", "top" });
    settings.registerEnum("command.view.type", "url", { "url", "comments" });
    settings.registerEnum("command.view.form", "normal", { "normal", "mobile", "compact", "json" });

    // options for various lists/data that are printed
    settings.registerBool("render.list.url", false);
    settings.registerBool("render.list.votes", true);
    settings.registerBool("render.list.name", false);
    settings.registerUInt("render.list.title.length", 0);

    return settings;
}

arcc::Settings initSettings()
{
    auto settings = registerAllSettings();

    boost::filesystem::path configfile{ utils::getDefaultConfigFile() };
    if (boost::filesystem::exists(configfile))
    {
        settings.load(configfile.string());
    }
    else
    {
        settings.save(configfile.string());
    }

    return settings;
}
int main(int, char*[])
{
    using namespace arcc;

    std::cout << APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;
    std::cout << std::endl;

    auto settings = initSettings();

    auto consoleApp = std::make_unique<ConsoleApp>(settings);
    if (consoleApp->loadSession())
    {
        ConsoleApp::printStatus("saved session restored");
    }
    else
    {
        consoleApp->setRedditSession(std::make_shared<arcc::RedditSession>());
    }

    try
    {
        consoleApp->run();
        settings.save(utils::getDefaultConfigFile());
    }
    catch (const std::exception& ex)
    {
        std::cerr << "terminal error: " << ex.what() << std::endl;
    }

    return 0;
}
