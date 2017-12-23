// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>

namespace utils
{

std::string getOsString();

std::string getUserFolder();

void openBrowser(const std::string& url_str);

} // namespace
