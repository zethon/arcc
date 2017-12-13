// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

namespace arcc
{
namespace console
{

class TerminalManager final
{

public:
    TerminalManager();
    ~TerminalManager();

    void run();
};

} // namespace console
} // namespace arcc