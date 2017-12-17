// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include <stdexcept>
#include <string>
#include <sstream>
#include <cstring>

#include "WebClient.h"

const uint      DEFAULT_MAX_REDIRECTS	= 5;

namespace arcc
{

static size_t CURLwriter(char *data, size_t size, size_t nmemb, std::string *writerData)
{
    if (writerData == nullptr)
    {
        return 0;
    }

    writerData->append(data, size*nmemb);
    return size * nmemb;
}

CURLcode curlGlobalInit()
{
    return curl_global_init(CURL_GLOBAL_ALL);
}

WebClient::WebClient()
{
    static CURLcode __global = curlGlobalInit();
    (void)__global; // silence unused warnings

    _curl = curl_easy_init();

    curl_version_info_data *vinfo = curl_version_info(CURLVERSION_NOW);
    if (!(vinfo->features & CURL_VERSION_SSL))
    {
        throw WebClientError("CURL SSL support is required but not enabled");
    }

    // set up our writer
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, CURLwriter);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &_buffer);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _errbuf);
    
    // set the redirects and the max number
    curl_easy_setopt(_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(_curl, CURLOPT_MAXREDIRS, DEFAULT_MAX_REDIRECTS);

    // start cookie engine
    curl_easy_setopt(_curl, CURLOPT_COOKIEFILE, "");

    // SSL CONFIG: since PEM is default, we needn't set it for PEM
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);

    // need to disable this otherwise SSL does not work on Windows 7
    curl_easy_setopt(_curl, CURLOPT_SSL_ENABLE_ALPN, 0);
    
    // tell libcurl to redirect a post with a post after a 301, 302 or 303
    curl_easy_setopt(_curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

    // disable all curl's signal handling
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1L);

#ifdef _DEBUG
   curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1);
   curl_easy_setopt(_curl, CURLOPT_DEBUGFUNCTION, trace);
#endif        
}

WebClient::~WebClient()
{
    curl_easy_cleanup(_curl);
}

auto WebClient::doRequest(const std::string& url, const std::string& payload, WebClient::Method method)
    -> WebClient::Reply
{
    WebClient::Reply retval;

    // set the URL we're getting
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());

    if (method == Method::POST)
    {
        if (payload.size() > 0)
        {
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, payload.size());
            curl_easy_setopt(_curl, CURLOPT_COPYPOSTFIELDS, payload.c_str());
        }
        else
        {
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, 0L);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, nullptr);
        }
    }
    else
    {
        curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(_curl, CURLOPT_POST, 0L);
    }

    _buffer.clear();
    CURLcode result = curl_easy_perform(_curl);

    long status = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status);
    
    retval.status = status;

    if (result == CURLE_OK)
    {
        char *finalUrl;
        curl_easy_getinfo(_curl, CURLINFO_EFFECTIVE_URL, &finalUrl);

        retval.status = status;

        if (status == 200)
        {
            retval.finalUrl = finalUrl;
            retval.data = _buffer; // copy! :(
        }
    }
    else
    {
        std::stringstream ss;

        if (std::strlen(_errbuf) > 0)
        {
            ss << "Request error: " << _errbuf;
        }
        else
        {
            ss << "Request error: " << curl_easy_strerror(result);
        }

        throw WebClientError(ss.str());
    }

   return retval;
}

} // namespace arcc