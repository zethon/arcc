// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#include "SimpleArgs.h"

#include <regex>
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
    const std::regex dashReg { R"(^\-{1,2}[^\-])" };

    if (original.size() > 0)
    {
        _original = original;
    }

    _tokenVector = tokenizeArgs(_original);
    std::smatch regmatches;

    for (unsigned int i = 0; i < (unsigned int)_tokenVector.size(); i++)
    {
        const std::string& token = _tokenVector.at(i);

        if (std::regex_search(token, regmatches, dashReg))
        {
            boost::string_view key = token;

            // remove '-' and possibly '--'
            key.remove_prefix(1);
            if (key.at(0) == '-')
            {
                key.remove_prefix(1);
            }

             if (i < _tokenVector.size() - 1)
             {
                _named.insert(std::make_pair(key, ++i));
             }
             else
             {
                 // allow something like: myprogram --option=true --option2
                 // so that `--option2` doesn't require a value but is still a 
                 // named argument
                 _named.insert(std::make_pair(key, -1));
             }
        }
        else
        {
            _positionals.push_back(i);
        }
    }
}

std::size_t SimpleArgs::getPositionalCount() const 
{
    return _positionals.size();
}

std::string SimpleArgs::getPositional(unsigned int index) const 
{
    return _tokenVector.at(_positionals.at(index));
}

std::size_t SimpleArgs::getNamedCount() const 
{ 
    return _named.size(); 
}

std::string SimpleArgs::getNamedArgument(const boost::string_view& name) const 
{ 
    if (_named.at(name) >= 0)
    {
        return _tokenVector.at(_named.at(name)); 
    }

    return std::string{};
}

bool SimpleArgs::hasArgument(const std::string& name) const 
{ 
    return _named.find(name) != _named.end(); 
}

std::size_t SimpleArgs::getTokenCount() const
{
    return _tokenVector.size();
}

std::string SimpleArgs::getToken(unsigned int index) const
{
    return _tokenVector.at(index);
}

} // namespace