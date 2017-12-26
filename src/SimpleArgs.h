// Another Reddit Console Client
// Copyright (c) 2017, Adalid Claure <aclaure@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <map>

#include <boost/utility/string_view.hpp>

namespace arcc
{

class SimpleArgs
{
    std::string                         _original;
    std::vector<std::string>            _tokenVector;

    std::vector<unsigned int>           _positionals;
    std::map<std::string, unsigned int> _named;
    
public:
    SimpleArgs(const std::string& val)
        : _original(val)
    {
    }

    void parse(const std::string& = std::string());

    unsigned int getPositionalCount() const { return _positionals.size(); }
    std::string getPositional(unsigned int index) { return _tokenVector.at(_positionals.at(index)); }
    
    unsigned int getNamedCount() const { return _named.size(); }
    std::string getArgument(const std::string& name) { return _tokenVector.at(_named.at(name)); }
    bool hasArgument(const std::string& name) { return _named.find(name) != _named.end(); }
};

} // namespace