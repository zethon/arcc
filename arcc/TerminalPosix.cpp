// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include "Terminal.h"

#include <unistd.h>
#include <termios.h>

namespace arcc
{

Terminal::Terminal()
{
    // disable echo when a key is pressed and install
    // the keypressed handler
    termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

Terminal::~Terminal()
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

std::string Terminal::getLine()
{
    reset();

    bool done = false;
    while (!done)
    {
        const auto retpair = getChar();
        if (retpair.first)
        {
            switch (retpair.second)
            {
                default:
                {
                    privateDoChar(retpair.second);
                }
                break;

                case 0x0A: // enter
                    done = true;
                break;

                case 0x7f:
                case 0x08:
                    privateDoBackspace();
                break;
            }
        }
        else
        {
            _commandLine.clear();
            done = true;
        }
    }

    return _commandLine;
}

} // namespace arcc
