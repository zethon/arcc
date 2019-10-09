// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <rang.hpp>
#include <fmt/core.h>

#include "utils.h"
#include "SimpleArgs.h"
#include "ConsoleApp.h"

namespace arcc
{

ConsoleCommand::ConsoleCommand(const std::string& n,
                               const std::string& hlp,
                               const std::string& usage,
                               ConsoleCommand::Handler hdr)

    : helpMessage_(hlp),
      usage_(usage),
      handler_(hdr)
{
    boost::split(commandNames_, n, boost::is_any_of(","));
}

void ConsoleApp::printError(const std::string& error)
{
    std::cout 
        << rang::fg::red
        << rang::style::bold
        << "error: " 
        << rang::fg::reset
        << rang::style::reset
        << error
        << std::endl;
}

void ConsoleApp::printStatus(const std::string& status)
{
    std::cout 
        << rang::fg::magenta
        << rang::style::bold
        << status
        << rang::fg::reset
        << rang::style::reset
        << std::endl;
}

ConsoleApp::ConsoleApp()
{
    initTerminal();
    initSettings();
    initCommands();

    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_history";
    _history.setHistoryFile(sessionfile.string());
    _history.loadHistory(false);
}

void ConsoleApp::initTerminal()
{
    _terminal.onUpArrow.connect(
        [this]() 
        { 
            if (_history.up())
            {
                _terminal.clearLine();
                const auto& newcmd = _history.getCurrent();
                _terminal.setLine(newcmd);
                std::cout << newcmd << std::flush;
            }
        });


    _terminal.onDownArrow.connect(
        [this]() 
        { 
            if (_history.down())
            {
                _terminal.clearLine();
                const auto& newcmd = _history.getCurrent();
                _terminal.setLine(newcmd);
                std::cout << newcmd << std::flush;
            }
        });
}

void ConsoleApp::initCommands()
{
    addCommand("help", "enter help system", std::bind(&ConsoleApp::help, this, std::placeholders::_1));
    addCommand("go,g", "go to a subreddit", std::bind(&ConsoleApp::go, this, std::placeholders::_1));
    addCommand("list,l,ls", "list stuff", std::bind(&ConsoleApp::list, this, std::placeholders::_1));
    addCommand("view,v", "view a listed item's link or comments", std::bind(&ConsoleApp::view, this, std::placeholders::_1));
    addCommand("set", "set the value of a setting", std::bind(&ConsoleApp::setCommand, this, std::placeholders::_1));
    addCommand("settings", "settings options", std::bind(&ConsoleApp::settingsCommand, this, std::placeholders::_1));
    addCommand("history", "command history options", std::bind(&ConsoleApp::history, this, std::placeholders::_1));

    addCommand("whoami", "whoami", [this](const std::string&) { whoami(); });
    addCommand("login", "login",
        [this](const std::string&)
        {
            if (!this->getRedditSession())
            {
                OAuth2Login login;
                login.start(); // will block until the login results are returned

                if (login.loggedIn())
                {
                    this->setRedditSession(login.getRedditSession());
                    std::cout << "login successful " << utils::sentimentText(utils::Sentiment::POSITIVE) << std::endl;
                }
                else
                {
                    std::cout << "login denied " << utils::sentimentText(utils::Sentiment::NEGATIVE) << std::endl;
                }
            }
            else
            {
                std::cout << "you are already logged in " << utils::sentimentText(utils::Sentiment::POSITIVE) << std::endl;
            }
        });

    addCommand("logout", "logout of the current session",
        [this](const std::string&)
        {
            if (this->getRedditSession() != nullptr)
            {
                // set location to the root
                this->setLocation("/");

                // delete our session object
                this->resetSession();

                std::cout << "you have logged out " << utils::sentimentText(utils::Sentiment::NEGATIVE) << std::endl;
            }
            else
            {
                std::cout << "you are not logged in " << utils::sentimentText(utils::Sentiment::NEUTRAL) << std::endl;
            }
        });

    addCommand("q,quit,exit", "exit the program",
        [this](const std::string&)
        {
            this->doExitApp();
        });

    addCommand("about", "information about this arcc",
        [](const std::string&) { utils::openBrowser("https://github.com/zethon/arcc"); });

    addCommand("open,launch", "open a website in the default browser",
        [](const std::string& params)
        {
            if (params.size() > 0)
            {
                std::vector<std::string> strings;
                boost::split(strings, params, boost::is_any_of(" "));

                std::string url = strings.at(0);

                // TODO: Apple's `CFURLRef` will want a properly formatted URL, so if the user enters
                // www.google.com, that will not be good enough. This is a naive solution to take
                // what the user entered and turn it into a proper URL. Improve this.
                if (!boost::starts_with(url,"http://") && !boost::starts_with(url,"https://"))
                {
                    url = "http://" + url;
                }

                utils::openBrowser(url);
            }
            else
            {
                // show useage
                std::cout << "Usage: open <url>" << std::endl;
            }
        });

    addCommand("ping", "ping a website",
        [](const std::string& params)
        {
            try
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                WebClient client;
                auto result = client.doRequest(params);
                auto t_end = std::chrono::high_resolution_clock::now();
                std::cout << result.data.size() << " bytes in " << std::chrono::duration<double, std::milli>(t_end-t_start).count() << " ms\n";
            }
            catch (WebClientError& e)
            {
                std::cout << "Could not ping because '" << e.what() << "'" << std::endl;
            }
        });

    addCommand("time", "print the current epoch time",
        [](const std::string&)
        {
            std::cout << std::time(nullptr) << std::endl;
        });
}

void ConsoleApp::initSettings()
{
    boost::filesystem::path homefolder{ utils::getUserFolder() };
    boost::filesystem::path configfile = homefolder / ".arcc_config";
    if (boost::filesystem::exists(configfile))
    {
        std::ifstream in(configfile.string());
        in >> _settings;
        in.close();
    }
    else
    {
        defaultSettings();
        saveSettings();
    }
}

void ConsoleApp::saveSettings()
{
    boost::filesystem::path homefolder{ utils::getUserFolder() };
    boost::filesystem::path configfile = homefolder / ".arcc_config";

    boost::filesystem::ofstream out;
    out.open(configfile.string(),
        boost::filesystem::ofstream::out | boost::filesystem::ofstream::trunc);

    out << _settings;
    out.close();
}

void ConsoleApp::defaultSettings()
{
    _settings.clear();

    // create the default config
    _settings["command.list.limit"] = 5;
    _settings["command.list.type"] = "hot";
    _settings["command.view.type"] = "url";
}

void ConsoleApp::exec(const std::string& rawline)
{
    bool executed = false;

    std::cout << std::endl;

    std::string command { rawline };
    std::string params;

    boost::algorithm::trim(command);
    auto firstspace = command.find(' ');
    if (firstspace != std::string::npos)
    {
        params = command.substr(firstspace + 1);
        command = command.substr(0, firstspace);
    }

    for (auto& c : _commands)
    {
        for (auto& alias : c.commandNames_)
        {
            if (alias == command)
            {
                try
                {
                    // TODO: optimize out the std::string()
                    c.handler_(params);
                    _history.commit(rawline);
                }
                catch (const std::exception& ex)
                {
                    std::cout << ex.what() << std::endl;
                }
                
                executed = true;
                goto endloop;
            }
        }
    }

    endloop:
    if (!executed)
    {
        std::cout << "Invalid command: " << command << std::endl;
    }
}

void ConsoleApp::run()
{
    while (!_doExit)
    {
        printPrompt();
        std::string line = _terminal.getLine();
        
        if (line.size() > 0)
        {
            exec(line);
        }
        else
        {
            std::cout << std::endl;
        }
    }

    if (_reddit)
    {
        try
        {
            saveSession();
        }
        catch (const std::exception& ex)
        {
            printError(ex.what());
        }
    }
}

bool ConsoleApp::setLocation(const std::string& location)
{
    bool retval = false;

    if (location == "/")
    {
        _location.clear();
        retval = true;
    }
    else
    {
        const std::regex subRegex { R"(^\/r\/[a-zA-Z0-9]+$)" };

        if (std::regex_match(location, subRegex))
        {
            const std::string jstr = doRedditGet(location + "/about");
            if (jstr.size() > 0)
            {
                try
                {
                    auto jreply = nlohmann::json::parse(jstr);
                    if (jreply["data"]["created"].get<unsigned int>() > 0)
                    {
                        retval = true;
                        _location = location;
                    }
                }
                catch (const nlohmann::json::exception&)
                {
                }
            }
        }
    }
    
    return retval;
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint)
{
    return doRedditGet(endpoint, RedditSession::Params{});
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint, const RedditSession::Params& params)
{
    std::string retval;

    if (_reddit)
    {
        retval = _reddit->doGetRequest(endpoint, params);
    }
    else
    {
        std::cout << "you must be logged in first. type `login` to sign into reddit" << std::endl;
    }

    return retval;
}

std::string ConsoleApp::doSubRedditGet(const std::string& endpoint)
{
    return doSubRedditGet(endpoint, RedditSession::Params{});
}

std::string ConsoleApp::doSubRedditGet(const std::string& endpoint, const RedditSession::Params& params)
{
    std::string retval;

    if (_reddit)
    {
        retval = _reddit->doGetRequest(_location + endpoint, params);
    }
    else
    {
        std::cout << "you must be logged in first. type `login` to sign into reddit" << std::endl;
    }

    return retval;
}

bool ConsoleApp::loadSession()
{
    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_session";

    if (boost::filesystem::exists(sessionfile))
    {
        std::ifstream i(sessionfile.string());

        try
        {
            nlohmann::json j;
            i >> j;

            if (j.find("accessToken") != j.end() && j.find("refreshToken") != j.end() && j.find("expiry") != j.end())
            {
                setRedditSession(std::make_shared<RedditSession>(
                    j["accessToken"].get<std::string>(), 
                    j["refreshToken"].get<std::string>(), 
                    j["expiry"].get<double>(),
                    j["time"].get<time_t>()));

                    _location = j["location"].get<std::string>();
            }
        }
        catch (const nlohmann::json::exception& )
        {
            // TODO: should warn the user here
            resetSession();
        }
    }

    return _reddit != nullptr;
}

void ConsoleApp::saveSession()
{
    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_session";

    nlohmann::json j;

    j["accessToken"] = _reddit->accessToken();
    j["refreshToken"] = _reddit->refreshToken();
    j["expiry"] = _reddit->expiry();
    j["time"] = _reddit->lastRefresh();
    j["location"] = _location;

    std::ofstream out(sessionfile.string());
    out << j;
    out.close();
}

void ConsoleApp::resetSession()
{
    _reddit.reset(); 

    boost::filesystem::path homefolder { utils::getUserFolder() };
    boost::filesystem::path sessionfile = homefolder / ".arcc_session";
    std::ofstream out(sessionfile.string());
    out << nlohmann::json{};
    out.close();
}

void ConsoleApp::printPrompt() const
{
    if (isLoggedIn())
    {
        std::cout
            << rang::fg::cyan
            << '$'
            << _location
            << rang::fg::reset
            << rang::bg::reset
            << rang::style::reset
            << "> ";
    }
    else
    {
        std::cout
            << rang::style::bold
            << rang::fg::red
            << '$'
            << _location
            << rang::style::reset
            << rang::fg::reset
            << "> ";
    }

    std::cout << std::flush;
}

void ConsoleApp::list(const std::string& cmdParams)
{
    static const std::vector<std::string> validTypes = {"new", "hot", "rising", "controversial", "top"};

    SimpleArgs args { cmdParams };

    std::string subReddit;
    if (args.hasArgument("sub"))
    {
        subReddit = args.getNamedArgument("sub");
    }

    std::string listType = "hot";
    if (args.getPositionalCount() > 0)
    {
        listType = args.getPositional(0);
        if (std::find(std::begin(validTypes), std::end(validTypes), listType) == std::end(validTypes))
        {
            ConsoleApp::printError("invalid list type '" + listType + "'");
            return;
        }
    }
    else
    {
        listType = _settings.value("command.list.type", listType);
    }

    std::uint16_t limit = 5;
    if (args.hasArgument("limit"))
    {
        auto limitStr = args.getNamedArgument("limit");
        try
        {
            limit = boost::lexical_cast<unsigned int>(limitStr);
        }
        catch (const boost::bad_lexical_cast&)
        {
            ConsoleApp::printError("parameter 'limit' has invalid value '" + limitStr + "'");
            return;
        }
    }
    else
    {
        limit = _settings.value("command.list.limit", limit);
    }

    RedditSession::Params params;
    params.insert(std::make_pair("limit", boost::lexical_cast<std::string>(limit)));

    ConsoleApp::printStatus(fmt::format("retrieving {} {} items from {}", 
        limit, listType, (subReddit.empty() ? "all" : subReddit)));
        
    auto jsontext = doSubRedditGet(subReddit + "/" + listType, params);
    if (jsontext.size() > 0)
    {
        const auto jreply = nlohmann::json::parse(jsontext);

        if (args.hasArgument("json"))
        {
            // roundtrip the JSON so we can get pretty indentation
            std::cout << jreply.dump(4) << std::endl;
        }
        else
        {
            unsigned int idx = 0;
            _lastObjects.clear();
            
            for (const auto& child : jreply["data"]["children"])
            {
                std::string flairText;
                try
                {
                    flairText = child["data"]["link_flair_text"].get<std::string>();
                }
                catch (const nlohmann::json::type_error&)
                {
                    // swallow this
                }

                if (flairText.size() > 0)
                {
                    flairText = "[" + flairText + "]";
                }

                if (child["data"]["stickied"].get<bool>())
                {
                    std::cout << rang::fg::black << rang::style::bold << rang::bg::yellow;
                }                

                std::cout 
                    << rang::style::bold
                    << ++idx
                    << ". "
                    << child["data"]["title"].get<std::string>()
                    << rang::style::reset
                    << '\n'
                    << rang::fg::cyan
                    << rang::style::underline
                    << child["data"]["url"].get<std::string>()
                    << rang::style::reset
                    << '\n'
                    << rang::fg::gray
                    << child["data"]["score"].get<int>() 
                    << " pts - "
                    << utils::miniMoment(child["data"]["created_utc"].get<std::uint32_t>())
                    << " - "
                    << child["data"]["num_comments"].get<int>() << " comments"
                    << '\n'
                    << rang::fg::magenta
                    << child["data"]["author"].get<std::string>()
                    << ' '
                    << rang::fg::yellow
                    << child["data"]["subreddit_name_prefixed"].get<std::string>();

                if (flairText.size() > 0)
                {
                    std::cout
                        << ' '
                        << rang::fg::red
                        << flairText;
                }

                std::cout
                    << rang::fg::reset
                    << rang::bg::reset
                    << rang::style::reset
                    << '\n'
                    << std::endl;

                _lastObjects.push_back(child);
            }
        }
    }
}

void ConsoleApp::whoami()
{
    auto jsontext = doRedditGet("/api/v1/me");

    if (jsontext.size() > 0)
    {
        auto jreply = nlohmann::json::parse(jsontext);

        std::cout << "username: " << jreply["name"].get<std::string>() << std::endl;

        std::time_t joined = jreply["created"].get<std::time_t>();
        std::cout << "joined  : " << std::asctime(std::localtime(&joined));
    }
}

void ConsoleApp::go(const std::string& params)
{
    SimpleArgs args { params };
    if (args.getTokenCount() > 0)
    {
        auto location = args.getToken(0);
        if (!boost::istarts_with(location, "/r/"))
        {
            location = "/r/" + location;
        }

        if (!setLocation(location))
        {
            ConsoleApp::printError("invalid subreddit '" + args.getToken(0) + "'");
        }
    }
    else
    {
       ConsoleApp::printError("no subreddit specified");
    }
}   

void ConsoleApp::view(const std::string& params)
{
    static const std::string usage = "usage: view <index> (-c,--comments | -u,--url)";

    if (SimpleArgs args{ params }; args.getPositionalCount() > 0)
    {
        std::string url;
        enum ViewType { UNKNOWN, COMMENTS, URL } viewType = UNKNOWN;

        if (const auto & firstarg = args.getPositional(0); utils::isNumeric(firstarg))
        {
            auto index = std::stoul(firstarg);

            if (index > 0 && index <= _lastObjects.size())
            {
                const auto& object = _lastObjects.at(index - 1);

                if (args.hasArgument("comments") || args.hasArgument("c"))
                {
                    viewType = COMMENTS;
                }
                else if (args.hasArgument("url") || args.hasArgument("u"))
                {
                    viewType = URL;
                }
                else if (args.getNamedCount() > 0)
                {
                    ConsoleApp::printError(usage);
                    return;
                }
                else
                {
                    if (_settings.value("command.view.type", "") == "comments")
                    {
                        viewType = COMMENTS;
                    }
                    else if (_settings.value("command.view.type", "") == "url")
                    {
                        viewType = URL;
                    }
                    else
                    {
                        ConsoleApp::printError(fmt::format("invalid 'command.view.type' settings. only 'comments' and 'url' allowed"));
                        return;
                    }
                }

                if (viewType == COMMENTS)
                {
                    url = "https://www.reddit.com" + object["data"].value("permalink", "");
                }
                else if (viewType == URL)
                {
                    url = object["data"].value("url", "");
                }
\
                utils::openBrowser(url);
            }
            else
            {
                ConsoleApp::printError("index out of range!");
            }
        }
        else
        {
            ConsoleApp::printError(usage);
        }
    }
    else
    {
        ConsoleApp::printError(usage);
    }
}

void ConsoleApp::setCommand(const std::string& params)
{
    if (params.size() == 0)
    {
        ConsoleApp::printError("usage: set <setting>=<value>");
        return;
    }

    using separator_type = boost::escaped_list_separator<char>;
    separator_type separator("\\", "= ", "\"\'");

    boost::tokenizer<separator_type> tokens(params, separator);

    // copy to a vector for convienence
    std::vector<std::string> result;
    std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
        [](const std::string& s) { return !s.empty(); });

    if (result.size() != 2)
    {
        ConsoleApp::printError("usage: set <setting>=<value>");
        return;
    }

    if (utils::isNumeric(result[1]))
    {
        _settings[result[0]] = std::stoul(result[1]);
    }
    else if (utils::isBoolean(result[1]))
    {
        _settings[result[0]] = utils::convertToBool(result[1]);
    }
    else
    {
        _settings[result[0]] = result[1];
    }

    saveSettings();
    ConsoleApp::printStatus(fmt::format("setting '{}' set to '{}'", result[0], result[1]));
}

void ConsoleApp::settingsCommand(const std::string& params)
{
    static const std::string usage = "usage: settings [list]";

    arcc::SimpleArgs args{params};

    if (args.getPositionalCount() != 1)
    {
        ConsoleApp::printError(usage);
        return;
    }

    const auto& command = args.getPositional(0);
    
    if (command == "list" || command.empty())
    {
        // find the longest key name for formatting
        std::size_t maxsize = 0;
        for (const auto& s : _settings.items())
        {
            maxsize = std::max(maxsize, s.key().size());
        }

        ConsoleApp::printStatus(fmt::format("{} setting(s)", _settings.size()));
        for (const auto& s : _settings.items())
        {
            std::cout
                << std::left
                << std::setw(static_cast<int>(maxsize + 5))
                << s.key()
                << " = "
                << s.value()
                << '\n';
        }
    }
    else if (command == "reset")
    {
        defaultSettings();
        saveSettings();
    }
}

void ConsoleApp::help(const std::string&)
{
    for (const auto& c : _commands)
    {
        std::cout
            << std::left
            << std::setw(15)
            << c.commandNames_.at(0)
            << " - "
            << c.helpMessage_
            << '\n';
    }

    std::cout << std::flush;
}

void ConsoleApp::history(const std::string& params)
{
    static const std::string usage = "usage: history [reset]";

    arcc::SimpleArgs args{params};
    if ((args.getPositionalCount() > 1)
        || (args.getPositionalCount() == 1 && args.getPositional(0) != "reset"))
    {
        ConsoleApp::printError(usage);
        return;
    }

    if (args.getPositionalCount() == 0)
    {
        std::uint16_t index = 0;
        for (const auto& c : _history)
        {
            std::cout
                << std::right
                << std::setw(5)
                << ++index
                << std::left
                << std::setw(5)
                << ' '
                << c
                << '\n';
        }

        std::cout << std::flush;
    }
    else
    {
        boost::filesystem::path homefolder{ utils::getUserFolder() };
        boost::filesystem::path configfile = homefolder / ".arcc_history";

        boost::filesystem::ofstream out;
        out.open(configfile.string(),
            boost::filesystem::ofstream::out | boost::filesystem::ofstream::trunc);
        out << std::endl;
        out.close();

        _history.clear();
    }
}

} // namespace arcc
