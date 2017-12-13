// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include "core.h"
#include "Console.h"

using namespace arcc;

int main(int argc, char* argv[])
{
    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    console::ConsoleHandler terminal;

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