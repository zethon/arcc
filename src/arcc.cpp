// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include "core.h"
#include "Console.h"

using namespace arcc;

class ConsoleState
{
    console::ConsoleHandler&    _terminalRef;
    std::string                 _commandLine;
    std::string::size_type      _currentPosition = 0;

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

    bool doEnter()
    {
        std::cout << std::endl;

        bool doExit = false;
        if (_commandLine == "quit")
        {
            doExit = true;
        }
        else
        {
            std::cout << "Current Command: " << _commandLine << std::endl;

            _commandLine.clear();
            _currentPosition = 0;

            std::cout << "> " << std::flush;
        }

        return doExit;
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

int main(int argc, char* argv[])
{
    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    console::ConsoleHandler terminal;
    ConsoleState conStat(terminal);

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