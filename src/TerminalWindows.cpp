// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <conio.h>

#include "Terminal.h"

namespace arcc
{
namespace console
{

Terminal::Terminal() = default;
Terminal::~Terminal() = default;

std::string Terminal::getLine()
{
    reset();

    bool done { false };
    while (!done)
    {
        const int c = _getch();

        switch (c)
        {
            default:
                privateDoChar(c);
            break;

            case '\r':
                done = true;
            break;

            case '\b':
                privateDoBackspace();
            break;

            //case 0:
            //case 0xe0:
            //{
            //    const int escchar = _getch();
            //    switch (escchar)
            //    {
            //        default:
            //        break;

            //        case 83:
            //            onDelete();
            //        break;
            //    }
            //}
            //break;
        }
    }

    return _commandLine;
}

} // namespace console
} // namespace arcc