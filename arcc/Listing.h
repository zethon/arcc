// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <memory>

#include <nlohmann/json.hpp>

namespace arcc
{

class RedditSession;
using SessionPtr = std::weak_ptr<RedditSession>;

class Listing;
using ListingPtr = std::unique_ptr<Listing>;

using Params = std::map<std::string, std::string>;

class Listing
{
    friend class RedditSession;

    SessionPtr                  _sessionPtr;

    const std::string           _endpoint;
    const std::size_t           _limit;

    std::string                 _after;
    std::string                 _before;


    std::size_t                 _count;

    nlohmann::json              _response;
    nlohmann::json              _data;

    void initialize(nlohmann::json reponse);

public:

    using Items = std::optional<std::reference_wrapper<const nlohmann::json>>;

    Listing(const Listing& other);
    Listing(SessionPtr session,
            const std::string& endpoint,
            std::size_t limit);

    ListingPtr getNextPage() const;
    ListingPtr getPreviousPage() const;

    const nlohmann::json& results() const { return _response; }
    const nlohmann::json& data() const;
    const Items items() const;

    std::string endpoint() const { return _endpoint; }
    std::string after() const { return _after; }
    std::string before() const { return _before; }
    std::size_t limit() const { return _limit; }
    std::size_t count() const { return _count; }
};

} // namespace
