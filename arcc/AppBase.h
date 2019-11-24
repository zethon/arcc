// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <memory>

namespace arcc
{

class AppBase;
using AppBasePtr = std::unique_ptr<AppBase>;

class AppBase
{

public:
    virtual ~AppBase() = default;
    virtual void run() = 0;
    
};

} // namespace