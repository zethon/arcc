// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include "Console.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>

namespace arcc
{
namespace console
{

ConsoleHandler::ConsoleHandler()
{
    // disable echo when a key is pressed and install
    // the keypressed handler
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

ConsoleHandler::~ConsoleHandler()
{
    // now restore the default behavior
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

std::pair<bool, char> getChar()
{
    using namespace std;

    bool bSuccess = false;
    char retchar;

    // get a keystroke
    while (true)
    {
        if (cin.fail())
        {
            break;
        }

        retchar = cin.get();
        bSuccess = true;
        break;
    }

    return std::make_pair(bSuccess, retchar);
}

void ConsoleHandler::run()
{
    bool done = false;

    // TODO: refactor to get rid of the `done` bool
    while (!done)
    {
        const auto retpair = getChar();
        if (retpair.first)
        {
            switch (retpair.second)
            {
                default:
                    onChar(retpair.second);
                break;

                case 0x0A:
                    // the idea here is to return true if we're done entering a command(?)
                    done = onEnter(); 
                break;

                case 0x7f:
                case 0x08:
                    onBackSpace();
                break;
            }
        }
        else
        {
            done = true;
        }
    }
}

void ConsoleHandler::echoBackspace()
{
    std::cout << "\b \b" << std::flush;
}

} // namespace console
} // namespace arcc