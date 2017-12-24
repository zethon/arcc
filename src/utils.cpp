// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include "utils.h"

#include <stdexcept>
#include <algorithm>
#include <functional>
#include <boost/tokenizer.hpp>

#ifdef _WINDOWS
#   include <windows.h>
#   include <shellapi.h>
#elif defined(__APPLE__)
#   include <CoreFoundation/CFBundle.h>
#   include <ApplicationServices/ApplicationServices.h>
#   include <unistd.h>
#   include <sys/types.h>
#   include <pwd.h>
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
    throw NotImplementedException();
#elif defined(__APPLE__)    
struct passwd *pw = getpwuid(getuid());
retval = pw->pw_dir;
#elif defined(__linux__)
    throw NotImplementedException();
#else
    throw NotImplementedException();
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

// https://stackoverflow.com/questions/18378798/use-boost-program-options-to-parse-an-arbitrary-string
/// @brief Tokenize a string.  The tokens will be separated by each non-quoted
///        space or equal character.  Empty tokens are removed.
///
/// @param input The string to tokenize.
///
/// @return Vector of tokens.
std::vector<std::string> tokenize(const std::string& input)
{
  typedef boost::escaped_list_separator<char> separator_type;
  separator_type separator("\\",    // The escape characters.
                           "= ",    // The separator characters.
                           "\"\'"); // The quote characters.

  // Tokenize the intput.
  boost::tokenizer<separator_type> tokens(input, separator);

  // Copy non-empty tokens from the tokenizer into the result.
  std::vector<std::string> result;
  std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(result), 
          [](const std::string& s) { return !s.empty(); });
  return result;
}

} // namespace
