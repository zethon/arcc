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
    bool done = false;
    _commandline.clear();

    while (!done)
    {
        const auto retpair = getChar();
        if (!retpair.first) break;

        switch (retpair.second)
        {
            default:
                if (auto op = onChar(retpair.second); 
                    op == boost::none || !*op)
                {
                    _commandline += retpair.second;
                    std::cout << retpair.second << std::flush;
                }
            break;

            case 0x0a: // enter
                if (auto op = onEnter(); 
                    op == boost::none || !*op)
                {
                    done = true;
                }
            break;

            case 0x7f:
            case 0x08:
                if (auto op = onBackspace(); 
                    (op == boost::none || !*op) && _commandline.size() > 0)
                {
                    _commandline.resize(_commandline.size () - 1);
                    backspace();
                }
            break;

            case 72:
                this->onHome();
            break;

            case 70:
                this->onEnd();
            break;

            case 21: // CTRL-U
                if (auto op = onClearLine(); 
                    op == boost::none || !*op)
                {
                    backspaces(_commandline.size());
                    _commandline.clear();
                }
            break;

            case 23: // CTRL-W
                this->onDeleteWord();
            break;

            case 0x1b: // ESC
            {
                if (std::cin.get() == '[')
                {
                    switch (std::cin.get())
                    {
                        default:
                        break;

                        case 'A':
                            this->onUpArrow();
                        break;

                        case 'B':
                            this->onDownArrow();
                        break;

                        case 'C':
                            this->onRightArrow();
                        break;

                        case 'D':
                            this->onLeftArrow();
                        break;

                    }
                }
            }
            break;
        }
    }

    return _commandline;
}

void Terminal::backspace()
{
    std::cout << "\b \b" << std::flush;
}

} // namespace arcc
