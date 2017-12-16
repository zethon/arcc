// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <stdexcept>

#include <curl/curl.h>

namespace arcc
{

class WebClientError : public std::runtime_error
{

long _status = -1;

public:
    using std::runtime_error::runtime_error;
};

class WebClient
{
    CURL*               _curl;                                  // the curl object
    std::string         _buffer;                                // buffer for response text
    char                _errbuf[CURL_ERROR_SIZE];               // detailed error buffer

public:
    struct Reply
    {
        std::string data;
        std::string finalUrl;
        long        status = -1;
    };

    enum class Method
    {
        GET = 1,
        POST = 2
    };

    WebClient();
    virtual ~WebClient();

    WebClient::Reply doRequest(const std::string& url, const std::string& payload, Method method);

};

} // namespace arcc