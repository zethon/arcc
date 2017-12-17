// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include "Console.h"

#include <conio.h>

namespace arcc
{
namespace console
{

ConsoleHandler::ConsoleHandler() = default;
ConsoleHandler::~ConsoleHandler() = default;

void ConsoleHandler::run()
{
    bool done { false };

    while (!done)
    {
        const int c = _getch();

        switch (c)
        {
            default:
                onChar(c);
            break;

            case '\r':
                done = onEnter();
            break;

            case '\b':
                onBackSpace();
            break;

            case 0:
            case 0xe0:
            {
                const int escchar = _getch();
                switch (escchar)
                {
                    default:
                    break;

                    case 83:
                        onDelete();
                    break;
                }
            }
            break;
        }
    }
}

void ConsoleHandler::echoBackspace()
{
    std::cout << "\b \b" << std::flush;
}

} // namespace console
} // namespace arcc