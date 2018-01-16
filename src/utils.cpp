// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include "utils.h"

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <ctime>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>

#ifdef _WINDOWS
#   include <windows.h>
#   include <shellapi.h>
#   include <Shlobj.h>
#else
#   include <unistd.h>
#   include <sys/types.h>
#   include <pwd.h>
#endif

#ifdef __APPLE__
#   include <CoreFoundation/CFBundle.h>
#   include <ApplicationServices/ApplicationServices.h>
#endif

namespace utils
{

class NotImplementedException : public std::logic_error
{
public:
    NotImplementedException()
    : std::logic_error("Function not yet implemented.")
    {
        // nothing to do
    }
};

std::string getOsString()
{
#ifdef _WINDOWS
    return "windows";
#elif defined(__APPLE__)    
    return "macos";
#elif defined(__linux__)
    return "linux";
#else
    return "unknown"
#endif
}

std::string getUserFolder()
{
    std::string retval;

#ifdef _WINDOWS
    WCHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path)))
    {
        std::wstring temp(path);
        retval.assign(temp.begin(), temp.end());
    }
    else
    {
        throw std::runtime_error("could not retrieve user folder");
    }
#else    
struct passwd *pw = getpwuid(getuid());
retval = pw->pw_dir;
#endif

    return retval;
}
    
void openBrowser(const std::string& url_str)
{
#ifdef _WINDOWS
    ShellExecute(0, 0, url_str.c_str(), 0, 0, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    // only works with `http://` prepended
    CFURLRef url = CFURLCreateWithBytes (
        NULL,                        // allocator
        (UInt8*)url_str.c_str(),     // URLBytes
        url_str.length(),            // length
        kCFStringEncodingASCII,      // encoding
        NULL                         // baseURL
    );

    LSOpenCFURLRef(url,0);
    CFRelease(url);
#elif defined(__linux__)
    throw NotImplementedException();
#else
    throw NotImplementedException();
#endif
}

std::string miniMoment(unsigned int stamp)
{
    std::stringstream os;

    std::chrono::time_point<std::chrono::system_clock> p2 = std::chrono::system_clock::now();
    auto nowt = std::chrono::duration_cast<std::chrono::seconds>(p2.time_since_epoch()).count();
    auto duration = nowt - stamp;

    if (duration < 60)
    {
        os << duration << "s";
    }
    else if (duration < (60 * 60))
    {
        os << std::ceil(duration / 60) << "m";
    }
    else if (duration < 60 * 60 * 24)
    {
        os << std::ceil(duration / (60 * 60)) << "h";
    }
    else if (duration < (60 * 60 * 24 * 365))
    {
        os << std::ceil(duration / (60 * 60 * 24)) << "d";
    }
    else
    {
        os << std::ceil(duration / (60 * 60 * 24 * 365)) << "y";
    }
    
    return os.str();
}

} // namespace
