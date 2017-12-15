// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <boost/algorithm/string.hpp>

#include "libs/cxxopts.hpp"

#include "core.h"
#include "Console.h"

#include "arcc.h"

namespace arcc
{
namespace console
{

using ConsoleAppPtr = std::unique_ptr<ConsoleApp>;
ConsoleAppPtr consoleApp;

bool executeCommand(const std::string&);

class ConsoleState
{
    console::ConsoleHandler&    _terminalRef;
    std::string                 _commandLine;
    std::string::size_type      _currentPosition = 0;
    bool                        _doExit = false;

public:        
    ConsoleState(console::ConsoleHandler& terminal)
        : _terminalRef(terminal)
    {
        std::cout << "> " << std::flush; // initial prompt
    }

    void doChar(char c)
    {
        _commandLine.insert(_currentPosition, 1, c);
        std::cout << c << std::flush;

        _currentPosition += 1;
    }

    void setDoExit(bool val)
    {
        _doExit = val;
    }

    bool doEnter()
    {
        std::cout << std::endl;

        executeCommand(_commandLine); // could set _doExit=true

        _commandLine.clear();
        _currentPosition = 0;

        if (!_doExit)
        {
            std::cout << "> " << std::flush;
        }

        return _doExit;
    }    

    void doBackSpace()
    {
        std::cout << "ConsoleState::doBackSpace()" << std::endl;
    } 

    void doDelete()
    {
        std::cout << "ConsoleState::doDelete()" << std::endl;
    }     
};

bool executeCommand(const std::string& rawcmd)
{
    bool success = false; // assume failure!

    std::string params;
    std::string cmd{rawcmd};
    
    boost::algorithm::trim(cmd);

    auto firstspace = cmd.find(' ');
    if (firstspace != std::string::npos)
    {
        params	= cmd.substr(firstspace + 1);
        cmd	= cmd.substr(0, firstspace);
    }

    consoleApp->exec(cmd, params);

    return success;
}

void initCommands()
{
    consoleApp->addCommand({"quit,exit", "exit the program", 
        [](const std::string& params)
        {
            consoleApp->invokeExitHandler();
        }});

    consoleApp->addCommand({"ping", "ping a website",
        [](const std::string& params)
        {
            std::cout << "ping!" << std::endl;
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

    consoleApp = std::unique_ptr<ConsoleApp>(new ConsoleApp());
    initCommands();

    ConsoleHandler terminal;
    ConsoleState conStat(terminal);

    consoleApp->setExitHandler([&conStat]() { conStat.setDoExit(true); });

    terminal.setCharHandler(boost::bind(&ConsoleState::doChar, &conStat, _1));
    terminal.setEnterHandler(boost::bind(&ConsoleState::doEnter, &conStat));
    terminal.setBackSpaceHandler(boost::bind(&ConsoleState::doBackSpace, &conStat));
    terminal.setDeleteHandler(boost::bind(&ConsoleState::doDelete, &conStat));

    try
    {
        terminal.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "terminal error: " << ex.what() << std::endl;
    }

    return 0;
}