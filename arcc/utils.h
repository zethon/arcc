// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <vector>

namespace utils
{

std::string getOsString();

std::string getUserFolder();

void openBrowser(const std::string& url_str);

std::string miniMoment(unsigned int stamp);

enum class Sentiment
{
    NEGATIVE,
    POSITIVE,
    NEUTRAL
};

std::string sentimentText(Sentiment s);

bool isNumeric(const std::string_view& s);

} // namespace
