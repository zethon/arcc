// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <conio.h>

#include "Terminal.h"

namespace arcc
{

Terminal::Terminal() = default;
Terminal::~Terminal() = default;

std::string Terminal::getLine()
{
    _commandline.clear();

    bool done { false };
    while (!done)
    {
        const int c = _getch();

        switch (c)
        {
            default:
                if (auto op = onChar(c);
                    op == boost::none || !*op)
                {
                    _commandline += c;
                    std::cout << static_cast<char>(c) << std::flush;
                }
            break;

            case '\r':
                if (auto op = onEnter();
                    op == boost::none || !*op)
                {
                    done = true;
                }
            break;

            case '\b':
                if (auto op = onBackspace();
                    (op == boost::none || !*op) && _commandline.size() > 0)
                {
                    _commandline.resize(_commandline.size() - 1);
                    backspace();
                }
            break;

            case 0:
            case 0xe0:
            {
                switch (_getch())
                {
                    default:
                    break;

                    case 72:
                        this->onUpArrow();
                    break;

                    case 80:
                        this->onDownArrow();
                    break;

                    case 75:
                        this->onLeftArrow();
                    break;

                    case 77:
                        this->onRightArrow();
                    break;
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