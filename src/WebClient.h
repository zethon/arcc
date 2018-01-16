// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <iostream>
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

    std::string         _authstr;                               // username:password for http basic auth
    
    // our own copy of the useragent since I believe Curl internally only stores the buffer
    std::string         _useragent;                             

public:
    struct Reply
    {
        std::string data;
        std::string finalUrl;
        long        status = -1;
    };


    friend std::ostream& operator<<(std::ostream& os, const WebClient::Reply& reply)
    {
        os << "{ " << reply.status << ", " << reply.finalUrl << ", " << reply.data << " }";
        return os;
    }

    enum class Method
    {
        GET = 1,
        POST = 2
    };

    WebClient();
    virtual ~WebClient();

    WebClient::Reply doRequest(const std::string& url, const std::string& payload = std::string(), Method method = Method::GET);

    void setBasicAuth(const std::string& username, const std::string& password)
    {
        _authstr = username + ":" + password;
        curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(_curl, CURLOPT_USERPWD, _authstr.c_str());
    }

    void setUserAgent(const std::string& useragent)
    {
        _useragent = useragent;
        curl_easy_setopt(_curl, CURLOPT_USERAGENT, _useragent.c_str());
    }

    void setHeader(const std::string& header)
    {
        // TODO: this only allows one custom header at a time
        curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, header.c_str());

        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);
    }

    void setTrace(bool trace);
};

} // namespace arcc