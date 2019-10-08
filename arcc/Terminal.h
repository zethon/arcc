// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <iostream>

namespace arcc
{

// proxy class for dealing with a specific OS's keyboard, the definitions
// reside in OS-specific .cpp files (ConsolePosix.cpp, ConsoleWindows.cpp)
class Terminal final
{
    std::string                 _commandLine;
    std::string::size_type      _currentPosition = 0;
    bool                        _bEcho = true;

public:
    Terminal();
    ~Terminal();

    std::string getLine();
    void setEcho(bool v) { _bEcho = v; }

private:
    void reset()
    {
        // reset the terminal input information
        _commandLine.clear();
        _currentPosition = 0;
    }

    void privateDoBackspace()
    {
        if (_currentPosition > 0)
        {
            std::cout << "\b \b" << std::flush;
            _currentPosition -= 1;
            _commandLine.erase(_currentPosition, 1);
        }
    }

    void privateDoChar(char c)
    {
        _commandLine.insert(_currentPosition, 1, c);
         std::cout << (_bEcho ? c : '*') << std::flush;
        _currentPosition += 1;
    }
};

} // namespace arcc
