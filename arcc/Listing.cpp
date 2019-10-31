// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <boost/algorithm/string.hpp>

#include <nlohmann/json.hpp>

#include "Listing.h"

namespace arcc
{

//std::string Listing::endpoint() const
//{
//    std::string retval;
//    if (!subreddit.empty() && subreddit != "/")
//    {
//        if (!boost::starts_with(subreddit, "/r"))
//        {
//            retval.append("/r");
//        }
//
//        retval.append(subreddit);
//    }
//
//    // some times a "/" can sneak into the subreddit
//    // name so check for it before creating a "//"
//    // which can be problematic
//    if (!boost::ends_with(retval, "/")) retval.append("/");
//
//    // now at `hot`, `new`, etc...
//    retval.append(type);
//    return retval;
//}

void Listing::reset()
{
    //subreddit.clear();
    //type.clear();
    //params.clear();
    //before.clear();
    //after.clear();
    //results.clear();
    //verbose = false;
    //details = false;
    //count = 0;
    //limit = 0;
}

}