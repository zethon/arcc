// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "Reddit.h"

namespace arcc
{

struct Listing
{
    std::string                 subreddit;
    std::string                 type;

    std::string                 before;
    std::string                 after;

    std::size_t                 count;
    std::size_t                 limit;

    RedditSession::Params       params;
    nlohmann::json              results;

    bool                        details;
    bool                        verbose;

    std::string endpoint() const;

    void reset();
};

} // namespace