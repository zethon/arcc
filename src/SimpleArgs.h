// Another Reddit Console Client
// Copyright (c) 2017-2018, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/utility/string_view.hpp>

namespace arcc
{

///
/// A simple parser that seperates positional and named arguments
///
class SimpleArgs
{
    std::string                                 _original;
    std::vector<std::string>                    _tokenVector;

    std::vector<unsigned int>                   _positionals;
    std::map<boost::string_view, int>  _named;
    
public:
    SimpleArgs(const std::string& val)
        : _original(val)
    {
        parse(_original);
    }

    void parse(const std::string& = std::string());

    std::size_t getPositionalCount() const;
    std::string getPositional(unsigned int index) const;
    
    std::size_t getNamedCount() const;
    std::string getNamedArgument(const boost::string_view& name) const;
    bool hasArgument(const std::string& name) const;

    std::size_t getTokenCount() const;
    std::string getToken(unsigned int index) const;
};

} // namespace