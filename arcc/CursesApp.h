// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <stdexcept>

//#ifdef _WINDOWS
//#   include <pdcurses.h>
//#elif defined(__APPLE__)    
//#   include <curses.h>
//#elif defined(__linux__)
//#   include <curses.h>
//#else
//#   error "ncurses is not supported on this system"
//#endif

#include <curses.h>

namespace arcc
{

WINDOW* curses_init();

} // namespace