// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <iostream>

#include <boost/signals2.hpp>

namespace arcc
{

// proxy class for dealing with a specific OS's keyboard, the definitions
// reside in OS-specific .cpp files (ConsolePosix.cpp, ConsoleWindows.cpp)
class Terminal final
{
    std::string _commandline;

public:
    Terminal();
    ~Terminal();

    [[nodiscard]]
    std::string getLine();
    void setLine(const std::string& val) { _commandline = val; }

    void backspace();
    void backspaces(std::size_t spaces)
    {
        for (auto i = 0u; i < spaces; i++)
        {
            backspace();
        }
    }

    void clearLine()
    {
        backspaces(_commandline.size());
        _commandline.clear();
    }

    boost::signals2::signal<bool(char)> onChar;
    boost::signals2::signal<bool()> onEnter;
    boost::signals2::signal<bool()> onBackspace;

    boost::signals2::signal<void()> onUpArrow;
    boost::signals2::signal<void()> onDownArrow;
    boost::signals2::signal<void()> onLeftArrow;
    boost::signals2::signal<void()> onRightArrow;

    boost::signals2::signal<void()> onHome;
    boost::signals2::signal<void()> onEnd;

    boost::signals2::signal<bool()> onClearLine;
    boost::signals2::signal<void()> onDeleteWord;
};

} // namespace arcc
