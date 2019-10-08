// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include <iostream>
#include <memory>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <nlohmann/json.hpp>
#include <rang.hpp>

#include "core.h"
#include "Terminal.h"
#include "WebClient.h"
#include "utils.h"
#include "SimpleArgs.h"

#include "ConsoleApp.h"

int main(int, char*[])
{
    using namespace arcc;

    std::cout <<  APP_TITLE << std::endl;
    std::cout << COPYRIGHT << std::endl;

    Terminal terminal;
    auto consoleApp = std::unique_ptr<ConsoleApp>(new ConsoleApp(terminal));

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
