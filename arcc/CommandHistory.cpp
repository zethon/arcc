#include <boost/filesystem.hpp>

#include <fmt/format.h>

#include "CommandHistory.h"


namespace arcc
{

void CommandHistory::loadHistory(bool throwOnError)
{
    if (_historyFile.empty()) return;

    boost::filesystem::ifstream in(_historyFile);
    in.imbue(std::locale::classic());
    if (in.fail())
    {
        if (throwOnError)
        {
            throw std::runtime_error(fmt::format("Cannot load file '{}'", _historyFile));
        }
        else
        {
            return;
        }
    }

    std::string line;

    while (!std::getline(in, line).fail())
    {
        if (!line.empty()) commit(line);
    }

    in.close();
}

void CommandHistory::saveHistory()
{
    if (_historyFile.empty()) return;

    boost::filesystem::ofstream out;
    out.open(_historyFile,
        boost::filesystem::ofstream::out | boost::filesystem::ofstream::trunc);

    if (!out.fail())
    {
        for (const auto& line : _buffer)
        {
            out << line << std::endl;
        }
    }

    out.close();
}

void CommandHistory::commit(const std::string &command)
{
    _buffer.push_back(command);
    _currentPos = _buffer.size();

    if (!_historyFile.empty())
    {
        boost::filesystem::ofstream out;
        out.open(_historyFile,
            boost::filesystem::ofstream::out | boost::filesystem::ofstream::app);

        if (!out.fail())
        {
            out << command;
            out.close();
        }
    }
}

bool CommandHistory::up()
{
    if (_currentPos > 0) 
    {
        _currentPos--;
        return true;
    }

    return false;
}

bool CommandHistory::down()
{
    if (_currentPos < _buffer.size()) 
    {
        _currentPos++;
        return true;
    }

    return false;
}

std::string CommandHistory::getCurrent() const
{
    if (_buffer.size() > 0)
    {
        return _buffer[_currentPos];
    }

    return std::string{};
}

} // namespace arcc
