// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/join.hpp>
#include <fmt/core.h>

#include "utils.h"

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <ctime>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <random>
#include <cctype>

#ifdef _WINDOWS
#   include <windows.h>
#   include <shellapi.h>
#   include <Shlobj.h>
#else
#   include <unistd.h>
#   include <sys/types.h>
#   include <pwd.h>
#   include <boost/process.hpp>
#endif

#ifdef __APPLE__
#   include <CoreFoundation/CFBundle.h>
#   include <ApplicationServices/ApplicationServices.h>
#endif

// There's some weirdness going on in Ubuntu where using the / operator
// on Ubuntu was throwing an error in some instances. Instead I set out
// to use boost::filesystem::path::seperator but that turned out to be
// a pain since it is multibyte on Windows! So I did this manually.
#ifdef _WINDOWS
#   define PATH_SEPERATOR   '\\'
#else
#   define PATH_SEPERATOR   '/'
#endif

namespace utils
{

NotImplementedException::NotImplementedException(const std::string& funcname)
    : std::logic_error(fmt::format("Function '{}' not yet implemented.", funcname))
{
    // nothing to do
}

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
    if (url_str.empty()) return;

#ifdef _WINDOWS
    ShellExecute(0, 0, url_str.c_str(), 0, 0, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    // only works with `http://` prepended
    CFURLRef url = CFURLCreateWithBytes (
        // allocator
        nullptr,

        // URLBytes
        (UInt8*)url_str.c_str(),     // URLBytes

        // length
        static_cast<std::int32_t>(url_str.length()),

        // encoding
        kCFStringEncodingASCII,

        // baseURL
        NULL
    );

    LSOpenCFURLRef(url, nullptr);
    CFRelease(url);
#elif defined(__linux__)
    boost::process::system("/usr/bin/xdg-open", url_str,
        boost::process::std_err > boost::process::null,
        boost::process::std_out > boost::process::null);
#else
    throw NotImplementedException("openBrowser");
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

std::string sentimentText(Sentiment s)
{
    std::string retval;

#ifdef _WINDOWS
    std::vector<std::string> negEmojis = 
        { "D8", ":(", "D:" };

    std::vector<std::string> posEmojis = 
        { "=)", ":D", ":)" };

    std::vector<std::string> neutralEmojis = 
        { ":-|", "-_-" };
#else
    std::vector<std::string> negEmojis = 
        { "(╯°□°）╯︵ ┻━┻", "(•̀o•́)ง" };

    std::vector<std::string> posEmojis = 
        { "ヽ(´▽`)/", "(/◔ ◡ ◔)/" };

    std::vector<std::string> neutralEmojis = 
        { "(•_•)" };
#endif

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

    switch (s)
    {
        default:
        break;

        case Sentiment::NEGATIVE:
        {
            std::uniform_int_distribution<> dis(0, static_cast<int>(negEmojis.size()) - 1);
            retval = negEmojis.at(static_cast<std::size_t>(dis(gen)));
        }
        break;

        case Sentiment::POSITIVE:
        {
            std::uniform_int_distribution<> dis(0, static_cast<int>(posEmojis.size()) - 1);
            retval = posEmojis.at(static_cast<std::size_t>(dis(gen)));
        }
        break;

        case Sentiment::NEUTRAL:
        {
            std::uniform_int_distribution<> dis(0, static_cast<int>(neutralEmojis.size()) - 1);
            retval = neutralEmojis.at(static_cast<std::size_t>(dis(gen)));
        }
        break;
    }

    return retval;
}

bool isNumeric(const std::string_view& s)
{
    return !s.empty() 
        && std::find_if(s.begin(), s.end(), 
            [](char c) 
            { 
                return !std::isdigit(c); 
            }) == s.end();
}


static const std::vector<std::string> trueStrings = { "true", "on", "1" };
static const std::vector<std::string> falseStrings = { "false", "off", "0" };

bool isBoolean(const std::string_view s)
{
    auto temp = boost::range::join(trueStrings, falseStrings);
    return std::find_if(std::begin(temp), std::end(temp),
        [s](const std::string& val) -> bool
        {
            return boost::iequals(val,s);
        })
        != std::end(temp);
}

bool convertToBool(const std::string_view s)
{
    if (std::find_if(
        std::begin(trueStrings), 
        std::end(trueStrings),
        [&s](const std::string& data)
        {
            return boost::iequals(data, s);
        }) != std::end(trueStrings))
    {
        return true;
    }
    else if (std::find_if(
        std::begin(falseStrings),
        std::end(falseStrings),
        [&s](const std::string& data)
        {
            return boost::iequals(data, s);
        }) != std::end(falseStrings))
    {
        return false;
    }

    throw std::runtime_error(fmt::format("invalid value '{}'", s));
}

std::string getDefaultHistoryFile()
{
    return fmt::format("{}{}{}",
        utils::getUserFolder(), PATH_SEPERATOR, ".arcc_history");
}

std::string getDefaultSessionFile()
{
    return fmt::format("{}{}{}",
        utils::getUserFolder(), PATH_SEPERATOR, ".arcc_session");
}

std::string getDefaultConfigFile()
{
    return fmt::format("{}{}{}",
        utils::getUserFolder(), PATH_SEPERATOR, ".arcc_config");
}


} // namespace
