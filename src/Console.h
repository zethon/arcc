// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <boost/signals2.hpp>

namespace arcc
{
namespace console
{

// proxy class for dealing with a specific OS's keyboard, the definitions
// reside in OS-specific .cpp files (ConsolePosix.cpp, ConsoleWindows.cpp)
class ConsoleHandler final
{

public:
    ConsoleHandler();
    ~ConsoleHandler();

    void run();

	template <class Handler>
	void setCharHandler(const Handler & h)
	{
		onChar.connect(h);
	}

	template <class Handler>
	void setEnterHandler(const Handler & h)
	{
		onEnter.connect(h);
	}     

	template <class Handler>
	void setBackSpaceHandler(const Handler & h)
	{
		onBackSpace.connect(h);
	}

	template <class Handler>
	void setDeleteHandler(const Handler & h)
	{
		onDelete.connect(h);
	}        

private:
    boost::signals2::signal<void(char)> onChar;
    boost::signals2::signal<bool(void), boost::signals2::last_value<bool> > onEnter;
    boost::signals2::signal<void(void)> onBackSpace;
    boost::signals2::signal<void(void)> onDelete;
};

} // namespace console
} // namespace arcc