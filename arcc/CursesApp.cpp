// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include "CursesApp.h"

//#define CATCHALL(handler) do { \
//		int nsig; \
//		for (nsig = SIGHUP; nsig < SIGTERM; ++nsig) \
//		    if (nsig != SIGKILL) \
//			signal(nsig, handler); \
//	    } while(0)
//
//#ifdef NCURSES_VERSION
//#define InitAndCatch(init,handler) do { CATCHALL(handler); init; } while (0)
//#else
//#define InitAndCatch(init,handler) do { init; CATCHALL(handler); } while (0)
//#endif

namespace arcc
{

WINDOW* curses_init()
{
    //InitAndCatch(initscr(), SIG_IGN);
    WINDOW * win = initscr();
    if (!win) 
    {
        throw std::runtime_error("initscr() failed to initialize curses");
    }
    
    return win;
}

void CursesApp::run()
{
        // [[maybe_unused]] auto window = arcc::curses_init();
        initscr();
        printw("Hi there!");
        // printw("hi there! %d", static_cast<void*>(window));
        getch();
        endwin();
}

} // namespace