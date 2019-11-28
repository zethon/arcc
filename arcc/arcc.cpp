// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "core.h"
#include "SimpleArgs.h"
#include "ConsoleApp.h"

namespace po = boost::program_options;
namespace po = boost::program_options;

arcc::Settings registerAllSettings()
{
    arcc::Settings settings;

    settings.registerBool("global.terminal.color", true);
    settings.registerEnum("global.mode", "text", { "text", "curses" });

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

std::shared_ptr<arcc::RedditSession> initSession()
{
    auto session = std::make_shared<arcc::RedditSession>();
    session->load(utils::getDefaultSessionFile());
    return session;
}

int main(int argc, char* argv[])
{
    using namespace arcc;

    setlocale(LC_ALL, "");

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,?", "print help message")
        ("version,v", "print version string")
    ;

    po::variables_map vm;

    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch(const po::error& er)
    {
        ConsoleApp::printError(er.what());
        return 1;
    }

    if (vm.count("help")) 
    {
        std::cout << desc << "\n";
        return 0;
    }
    else if (vm.count("version"))
    {
        std::cout << APP_TITLE << std::endl;
        std::cout << COPYRIGHT << std::endl;
        return 0;
    }

    auto settings = initSettings();
    auto session = initSession();

    std::cout << APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;
    std::cout << std::endl;

    try
    {
        auto consoleApp = std::make_unique<ConsoleApp>(settings, session);
        consoleApp->run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "terminal error: " << ex.what() << std::endl;
    }

    settings.save(utils::getDefaultConfigFile());

    return 0;
}
