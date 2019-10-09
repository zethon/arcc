// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iostream>

#include "core.h"
#include "Terminal.h"
#include "ConsoleApp.h"

int main(int, char*[])
{
    using namespace arcc;

    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    auto consoleApp = std::unique_ptr<ConsoleApp>(new ConsoleApp());
    if (consoleApp->loadSession())
    {
        ConsoleApp::printStatus("saved session restored");
    }

    try
    {
        consoleApp->run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "terminal error: " << ex.what() << std::endl;
    }

    return 0;
}
