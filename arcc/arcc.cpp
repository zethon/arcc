// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <curses.h>

#include "core.h"
#include "SimpleArgs.h"
#include "AppBase.h"
#include "ConsoleApp.h"
#include "CursesApp.h"

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

int main(int argc, char* argv[])
{
    using namespace arcc;

    setlocale(LC_ALL, "");

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,?", "print help message")
        ("version,v", "print version string")
        ("mode,m", po::value<std::string>(), "run in 'curses' (default) or 'text' mode")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

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

    enum ModeEnum { NCURSES, TEXT } termMode = TEXT;
    if (vm.count("mode"))
    {
        const std::string mode = vm["mode"].as<std::string>();
        if (boost::iequals(mode, "curses")) 
        {   
            termMode = NCURSES;
        }
        else if (!boost::iequals(mode, "text"))
        {
            std::cout << "error: possible values for 'mode' are [curses|text]\n";
            return 1;
        }
    }
    else
    {
        const auto mode = settings.value("global.mode", "text");
        if (mode == "curses") termMode = NCURSES;
    }

    arcc::AppBasePtr app;

    if (termMode == NCURSES)
    {
        //// [[maybe_unused]] auto window = arcc::curses_init();
        //initscr();
        //printw("Hi there!");
        //// printw("hi there! %d", static_cast<void*>(window));
        //getch();
        //endwin();
        app = std::make_unique<arcc::CursesApp>();
    }
    else
    {
        app = std::make_unique<arcc::ConsoleApp>(settings);

        std::cout << APP_TITLE << std::endl;
        std::cout << COPYRIGHT << std::endl;
        std::cout << std::endl;

        //if (consoleApp->loadSession())
        //{
        //    ConsoleApp::printStatus("saved session restored");
        //}
    }

    try
    {
        app->run();
        settings.save(utils::getDefaultConfigFile());
    }
    catch (const std::exception& ex)
    {
        std::cerr << "error: " << ex.what() << std::endl;
    }
    

    return 0;
}
