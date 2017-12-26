// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#include "SimpleArgs.h"

#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

namespace arcc
{

// https://stackoverflow.com/questions/18378798/use-boost-program-options-to-parse-an-arbitrary-string
/// @brief Tokenize a string.  The tokens will be separated by each non-quoted
///        space or equal character.  Empty tokens are removed.
///
/// @param input The string to tokenize.
///
/// @return Vector of tokens.
std::vector<std::string> tokenizeArgs(const std::string& input)
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

void SimpleArgs::parse(const std::string& original)
{
    if (original.size() > 0)
    {
        _original = original;
    }

    _tokenVector = tokenizeArgs(_original);
    
    for (unsigned int i = 0; i < (unsigned int)_tokenVector.size(); i++)
    {
        // const std::string& token = _tokenVector.at(i);
        // if (boost::starts_with(token, "-") ||)
    }
}

} // namespace