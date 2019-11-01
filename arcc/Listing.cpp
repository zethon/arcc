// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include "Listing.h"

namespace arcc
{

Listing::Listing(SessionPtr session, const std::string& endpoint)
    : _sessionPtr{ session },
      _endpoint{ endpoint }
{}

Listing Listing::getNextPage() const
{
    return Listing{ _sessionPtr, _endpoint };
}


Listing Listing::getPreviousPage() const
{
    return Listing{ _sessionPtr, _endpoint };
}

}