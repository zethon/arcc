// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <stdexcept>

#include <curses.h>

#include "AppBase.h"

namespace arcc
{

WINDOW* curses_init();

class CursesApp final : public AppBase
{

public:
    void run() override;

};

} // namespace