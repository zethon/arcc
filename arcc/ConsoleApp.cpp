// Another Reddit Console Client
// Copyright (c) 2017-2019, Adalid Claure <aclaure@gmail.com>

#include <iomanip>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/join.hpp>

#include <rang.hpp>
#include <fmt/core.h>
#include <nlohmann/json_fwd.hpp>

#include "utils.h"
#include "SimpleArgs.h"
#include "OAuth2Login.h"
#include "core.h"
#include "Settings.h"

#include "ConsoleApp.h"

using namespace std::string_literals;

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
        << "-- "
        << "error: " 
        << rang::fg::reset
        << rang::style::reset
        << error
        << std::endl;
}

void ConsoleApp::printWarning(const std::string& warning)
{
    std::cout
        << rang::fg::yellow
        << rang::style::bold
        << "-- "
        << warning
        << rang::fg::reset
        << rang::style::reset
        << std::endl;
}

void ConsoleApp::printStatus(const std::string& status)
{
    std::cout 
        << rang::fg::magenta
        << rang::style::bold
        << "-- "
        << status
        << rang::fg::reset
        << rang::style::reset
        << std::endl;
}

ConsoleApp::ConsoleApp(arcc::Settings& settings, std::shared_ptr<arcc::RedditSession> session)
    : _settings{ settings },
      _session{ session }
{
    initTerminal();
    initCommands();

    const std::string historyfile{ utils::getDefaultHistoryFile() };
    _history.setHistoryFile(historyfile);
    _history.load(false);

    _session->setRefreshCallback(
        [this]() { _session->save(utils::getDefaultSessionFile()); });
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
    addCommand("set", "set the value of a setting", std::bind(&ConsoleApp::setCommand, this, std::placeholders::_1));
    addCommand("settings", "settings options", std::bind(&ConsoleApp::settingsCommand, this, std::placeholders::_1));
    addCommand("history", "command history options", std::bind(&ConsoleApp::history, this, std::placeholders::_1));

    addCommand("go,g,cd", "go to a subreddit", std::bind(&ConsoleApp::go, this, std::placeholders::_1));
    addCommand("view,v", "view a listed item's link or comments", std::bind(&ConsoleApp::view, this, std::placeholders::_1));

    addCommand("list,l,ls", "list links and posts", std::bind(&ConsoleApp::list, this, std::placeholders::_1));
    addCommand("next,n", "go to the next page of links and posts", std::bind(&ConsoleApp::next, this, std::placeholders::_1));
    addCommand("previous,p", "go to a previous page of links and posts", std::bind(&ConsoleApp::previous, this, std::placeholders::_1));
    addCommand("current,c", "list items on the current page", 
        [this](const std::string&)
        {
            if (_currentPage.size() > 0)
            {
                printListing();
            }
            else
            {
                ConsoleApp::printWarning("there are no items on the current page");
            }
        });

    addCommand("whoami", "whoami", [this](const std::string&) { whoami(); });
    addCommand("login", "login",
        [this](const std::string&)
        {
            if (!_session->loggedIn())
            {
                OAuth2Login login;
                login.start(); // will block until the login results are returned

                if (login.loggedIn())
                {
                    _session = login.getRedditSession();
                    _session->save(utils::getDefaultSessionFile());
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
            if (_session->loggedIn())
            {
                _session->reset();
                _session->save(utils::getDefaultSessionFile());
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

    addCommand("about", "information about arcc",
        [](const std::string&) 
        { 
            std::cout << fmt::format("{} ({})\n", APP_TITLE, utils::getOsString());
            std::cout << COPYRIGHT << '\n';
            std::cout << fmt::format("build-date : {}\n", BUILDTIMESTAMP);
            std::cout << fmt::format("user-folder : {}\n", utils::getUserFolder());
            std::cout << std::endl;
        });

    addCommand("git", "load the github page for arcc",
        [](const std::string&) { utils::openBrowser(GITHUB_PAGE); });

    addCommand("issue,issues,bug,bugs,", "report an issue with arcc",
        [](const std::string&) { utils::openBrowser(fmt::format("{}/issues", GITHUB_PAGE)); });

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
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::cout << "time  : " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '\n';
            std::cout << "epoch : " << t << std::endl;
        });

    addCommand("last", "print the url of the last request",
        [this](const std::string&)
        {
            printInfo("last request: {}", _session->lastRequest());
        });

    addCommand("session", "print current session info",
        [this](const std::string&)
        {
            std::cout << "access token  : " <<
                (_session->accessToken().empty() ? "\"\""s : _session->accessToken()) << '\n';

            std::cout << "refresh token : " <<
                (_session->refreshToken().empty() ? "\"\""s : _session->refreshToken()) << '\n';

            std::cout << "expiry        : " << _session->expiry() << '\n';
            std::cout << "time          : " << _session->lastRefresh() << '\n';

            std::cout << "location      : " <<
                (_session->location().empty() ? "\"\""s : _session->location()) << '\n';
        });

    addCommand("dump", "dump the json of a listing item",
        [this](const std::string& params)
        {
            SimpleArgs args { params };
            if (args.getTokenCount() > 0)
            {
                if (auto firstArg = args.getToken(0);
                    utils::isNumeric(firstArg))
                {
                    std::size_t index = boost::lexical_cast<std::size_t>(firstArg);
                    if (index == 0 || (index > _currentPage.size()))
                    {
                        printError(fmt::format("invalid listing index: {}", index));
                    }
                    else
                    {
                        const auto& data = _currentPage.at(index - 1);
                        std::cout << std::setw(4) << data << '\n';
                    }
                }
            }
        });

}

// poor man's way of updating settings, would be better
// to optionally store a callback with each value and 
// detect when it's been changed
void ConsoleApp::refreshSettings()
{
    if (_settings.value("global.terminal.color", false))
    {
        rang::setControlMode(rang::control::Auto);
    }
    else
    {
        rang::setControlMode(rang::control::Off);
    }
}

void ConsoleApp::exec(const std::string& rawline)
{
    const static std::regex histre{ R"(^!(\d+)\s*(.*)$)" };
    bool executed = false;

    std::cout << std::endl;

    std::string historyCommand{ rawline };
    std::string command { rawline };
    std::string params;

    boost::algorithm::trim(command);

    if (std::smatch match; std::regex_match(command, match, histre)
        && match.size() > 1 && match[1].str().size() > 0)
    {
        assert(utils::isNumeric(match[1].str()));
        assert(match.size() == 3);

        auto index = std::stoul(match[1].str())-1;
        if (index >= _history.size())
        {
            ConsoleApp::printError(fmt::format("!{} event not found", match[1].str()));
            return;
        }
        
        historyCommand = command = fmt::format("{} {}",
            _history.at(index),
            match[2].str());

        // trim again in case we added any whitespace
        boost::algorithm::trim(command);

        // print the command we're executing
        std::cout << command << std::endl;
    }

    auto firstspace = command.find(' ');
    if (firstspace != std::string::npos)
    {
        params = command.substr(firstspace + 1);
        command = command.substr(0, firstspace);
    }

    if (utils::isNumeric(command))
    {
        std::size_t index = boost::lexical_cast<std::size_t>(command);
        openIndex(index);
    }
    else
    {
        for (auto& c : _commands)
        {
            for (auto& alias : c.commandNames_)
            {
                if (alias == command)
                {
                    try
                    {
                        c.handler_(params);
                    }
                    catch (const std::exception& ex)
                    {
                        ConsoleApp::printError(ex.what());
                    }

                    executed = true;
                    goto endloop;
                }
            }
        }

        endloop:
        if (!executed)
        {
            printError(fmt::format("invalid command: {}", command));
        }
    }

    if (!historyCommand.empty()) _history.commit(historyCommand);
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
}

bool ConsoleApp::setLocation(const std::string& location)
{
    bool retval = false;

    if (location == "/")
    {
        _session->setLocation(""s);
        retval = true;
    }
    else
    {
        static const std::regex subRegex { R"(^\/r\/[a-zA-Z0-9]+$)" };
        static const std::regex commentRegex { R"(^\/r\/[a-zA-Z0-9]+\/[a-zA-Z0-9]+$)" };

        if (std::regex_match(location, subRegex))
        {
            const std::string jstr = doRedditGet(location + "/about");
            if (jstr.size() > 0)
            {
                try
                {
                    auto jreply = nlohmann::json::parse(jstr);
                    if (jreply["data"]["created"].get<std::uint32_t>() > 0)
                    {
                        _session->setLocation(location);
                        retval = true;
                    }
                }
                catch (const nlohmann::json::exception& ex)
                {
                    printError(fmt::format("could no set location: {}", ex.what()));
                }
            }
        }
    }

    if (retval) _session->save(utils::getDefaultSessionFile());

    return retval;
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint)
{
    return doRedditGet(endpoint, Params{});
}

std::string ConsoleApp::doRedditGet(const std::string& endpoint, const Params& params)
{
    std::string retval;

    if (_session->loggedIn())
    {
        retval = _session->doGetRequest(endpoint, params);
    }
    else
    {
        std::cout << "you must be logged in first. type `login` to sign into reddit" << std::endl;
    }

    return retval;
}

void ConsoleApp::printPrompt() const
{
    if (_session->loggedIn())
    {
        std::cout
            << rang::fg::cyan
            << (_session->location().empty() ? "/"s : _session->location())
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
            << _session->location()
            << rang::style::reset
            << rang::fg::reset
            << "> ";
    }

    std::cout << std::flush;
}

void ConsoleApp::whoami()
{
    if (!_session->loggedIn())
    {
        printWarning("you must be logged in first");
        return;
    }

    if (const auto jsontext = doRedditGet("/api/v1/me");
        jsontext.size() > 0)
    {
        auto jreply = nlohmann::json::parse(jsontext);

        std::cout << "username: " << jreply["name"].get<std::string>() << std::endl;

        std::time_t joined = jreply["created"].get<std::time_t>();
        std::cout << "joined  : " << std::asctime(std::localtime(&joined));
    }
}

void ConsoleApp::openIndex(std::size_t index)
{
    namespace nl = nlohmann;

    if (!_listing)
    {
        printError("there is no active list");
    }
    else if (index == 0 || (index > _currentPage.size()))
    {
        printError(fmt::format("invalid listing index: {}", index));
    }
    else
    {
        const auto& item = _currentPage.at(index - 1);
        const auto& data = item["data"];

        const auto endpoint =
            fmt::format("/r/{}/comments/{}", data["subreddit"], data["id"]);

        auto jsontext = doRedditGet(endpoint);
        nl::json items = nl::json::parse(jsontext);

        if (!items.is_array() || items.size() == 0)
        {
            printError("invalid response");
            return;
        }

        const auto text = data.value("selftext","");
        renderLink(data, 0, data.value("is_self", false) && !text.empty());

        std::cout << "(" << fmt::format("{}/{}", _session->location(), data["id"]) << ")\n";
//        setLocation();

        if (items.size() > 1)
        {
            const auto& commentlistobj = items.at(1);
            if (commentlistobj.value("kind","") != "Listing")
            {
                throw std::runtime_error("invalid 'kind");
            }

            const auto& listarray = commentlistobj["data"]["children"];
            if (!listarray.is_array())
            {
                printError("no children");
            }

            std::cout << fmt::format("there are {} comment children in the tree", listarray.size()) << std::endl;
        }
    }
}

void ConsoleApp::go(const std::string& params)
{
    SimpleArgs args { params };
    if (args.getTokenCount() > 0)
    {
        if (auto firstArg = args.getToken(0);
            utils::isNumeric(firstArg))
        {
            std::size_t index = boost::lexical_cast<std::size_t>(firstArg);
            openIndex(index);
        }
        else
        {
            auto location = firstArg;
            if (location != "/" &&
                !boost::istarts_with(location, "/r/"))
            {
                location = "/r/" + location;
            }

            if (!setLocation(location))
            {
                ConsoleApp::printError("invalid subreddit '" + firstArg + "'");
            }
            else if (_settings.value("command.go.autolist", (bool)false))
            {
                list(std::string{});
            }
        }
    }
    else
    {
       ConsoleApp::printError("no subreddit specified");
    }
}   

ConsoleApp::ViewFormType ConsoleApp::parserViewFormType(const std::string& val)
{
    if (boost::iequals(val, "normal")) return ViewFormType::NORMAL;
    else if (boost::iequals(val, "mobile")) return ViewFormType::MOBILE;
    else if (boost::iequals(val, "compact")) return ViewFormType::COMPACT;
    else if (boost::iequals(val, "json")) return ViewFormType::JSON;

    return ViewFormType::NORMAL;
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

            if (index > 0 && index <= _currentPage.size())
            {
                const auto& page = _currentPage.at(index-1);
                assert(page.find("data") != page.end());
                const auto& data = page.at("data");
                assert(data.find("url") != data.end());
                assert(data.find("permalink") != data.end());

                if (args.hasArgument("comments") || args.hasArgument("c"))
                {
                    viewType = COMMENTS;
                }
                else if (args.hasArgument("url") || args.hasArgument("u"))
                {
                    viewType = URL;
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
                        ConsoleApp::printError(fmt::format(
                            "invalid 'command.view.type' settings. only 'comments' and 'url' allowed"));

                        return;
                    }
                }

                if (viewType == COMMENTS)
                {
                    url = fmt::format("https://www.reddit.com{}", data["permalink"]);
                    
                    auto formType = ViewFormType::NORMAL;
                    if (args.hasArgument("mobile")) formType = ViewFormType::MOBILE;
                    else if (args.hasArgument("compact")) formType = ViewFormType::COMPACT;
                    else if (args.hasArgument("json")) formType = ViewFormType::JSON;
                    else
                    {
                        formType = parserViewFormType(_settings.value("command.view.form", "normal"s));
                    }
                    
                    if (formType != ViewFormType::NORMAL)
                    {
                        if (!boost::ends_with(url, "/")) url.append("/");
                        switch (formType)
                        {
                            default:
                            break;

                            case ViewFormType::MOBILE:
                                url.append(".mobile?keep_extension=True");
                            break;

                            case ViewFormType::COMPACT:
                                url.append(".compact");
                            break;

                            case ViewFormType::JSON:
                                url.append(".json");
                            break;
                        }
                    }
                }
                else if (viewType == URL)
                {
                    url = data["url"];
                }

                printStatus(fmt::format("opening '{}'", url));
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

    if (!_settings.exists(result[0]))
    {
        ConsoleApp::printError(fmt::format("unknown setting '{}'", result[0]));
        return;
    }

    _settings.set(result[0], result[1]);

    _settings.save(utils::getDefaultConfigFile());
    refreshSettings();
    ConsoleApp::printStatus(fmt::format("setting '{}' set to '{}'", result[0], result[1]));
}

void ConsoleApp::settingsCommand(const std::string& params)
{
    static const std::string usage = "usage: settings [list|rest]";

    arcc::SimpleArgs args{params};

    if (args.getPositionalCount() == 0
        || args.getPositional(0) == "list")
    {
        // find the longest key name for formatting
        std::size_t maxsize = 0;
        for (const auto& s : _settings)
        {
            maxsize = std::max(maxsize, s.key().size());
        }

        ConsoleApp::printStatus(fmt::format("{} setting(s)", _settings.size()));
        for (const auto& s : _settings)
        {
            std::cout
                << std::left
                << std::setw(static_cast<int>(maxsize + 2))
                << s.key()
                << " = "
                << s.value()
                << '\n';
        }
    }
    else if (args.getPositional(0) == "reset")
    {
        _settings.reset();
        _settings.save(utils::getDefaultConfigFile());
        printStatus(fmt::format("{} settings reset", _settings.size()));
    }
    else
    {
        ConsoleApp::printError(usage);
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
        for (auto index = 0u; index < _history.size(); index++)
        {
            std::cout
                << std::right
                << std::setw(5)
                << index + 1
                << std::left
                << std::setw(5)
                << ' '
                << _history.at(index)
                << '\n';
        }

        std::cout << std::flush;
    }
    else
    {
        const std::string historyfile{ utils::getDefaultHistoryFile() };

        boost::filesystem::ofstream out;
        out.open(historyfile,
            boost::filesystem::ofstream::out | boost::filesystem::ofstream::trunc);
        out << std::endl;
        out.close();

        _history.clear();
    }
}

void ConsoleApp::renderLink(const nlohmann::json& link, std::size_t idx, bool details)
{
    std::string flairText;
    if (link.find("link_flair_text") != link.end()
        && !link["link_flair_text"].is_null())
    {
        flairText = fmt::format("[{}]", link.at("link_flair_text"));
    }

    if (link["stickied"].get<bool>())
    {
        std::cout << rang::style::bold << rang::fg::yellow;
    }

    std::string namestr;
    if (_settings.value("render.list.name", false))
    {
        namestr = fmt::format(" ({})", link["id"]);
    }

    std::string updownstr;
    if (_settings.value("render.list.votes", false))
    {
        updownstr = fmt::format(" (+{}/-{})",
            std::to_string(link["ups"].get<std::uint32_t>()),
            std::to_string(link["downs"].get<std::uint32_t>()));
    }

    std::string urlstr;
    if (_settings.value("render.list.url", false))
    {
        urlstr = fmt::format("{}\n", link.value("url", ""));
    }

    std::string titlestr { link.value("title","")} ;
    if (const std::size_t maxlen = _settings.value("render.list.title.length",0u);
        maxlen > 0 && titlestr.size() > maxlen)
    {
        // first try trimming the string, there could be some 
        // extraneous white space at the end
        boost::algorithm::trim(titlestr);
        if (titlestr.size() > maxlen)
        {
            titlestr = titlestr.substr(0, maxlen - 3);
            titlestr.append("...");
        }
    }

    std::string idxstr;
    if (idx > 0) idxstr = fmt::format("{}. ", idx);

    std::string textstr;
    if (details) textstr = fmt::format("{}\n", link.value("selftext",""));

    std::string typetstr;
    if (link.value("is_self", false))
    {
        typetstr = "[s]";
    }
    else if (link.value("is_reddit_media_domain", false))
    {
        typetstr = "[m]";
    }
    else if (link.value("is_video", false))
    {
        typetstr = "[v]";
    }
    else if (!link.value("post_hint", "").empty())
    {
        const auto hint = link["post_hint"];
        if (hint == "link")
        {
            typetstr = "[l]";
        }
        else if (hint == "rich:video")
        {
            typetstr = "[V]";
        }
        else
        {
            typetstr = fmt::format("({})", link["post_hint"]);
        }
    }
    else
    {
        typetstr = "[?]";
    }

    std::cout
        << rang::style::bold
        << idxstr
        << utils::stripWideCharacters(titlestr)
        << rang::style::reset
        << rang::fg::reset
        << namestr
        << rang::style::reset
        << '\n'
        << rang::fg::cyan
        << rang::style::underline
        << urlstr
        << rang::style::reset
        << rang::fg::reset
        << utils::stripWideCharacters(textstr)
        << rang::style::reset
        << rang::fg::green
        << link["score"].get<int>()
        << " pts"
        << updownstr
        << " - "
        << utils::miniMoment(link["created_utc"].get<std::uint32_t>())
        << " - "
        << link["num_comments"].get<int>() << " comments"
        << '\n'
        << rang::style::reset
        << rang::fg::magenta
        << link["author"].get<std::string>()
        << rang::style::bold
        << rang::fg::cyan
        << typetstr
        << ' '
        << rang::style::reset
        << rang::fg::yellow
        << link["subreddit_name_prefixed"].get<std::string>();

    if (flairText.size() > 0)
    {
        std::cout
            << ' '
            << rang::style::bold
            << rang::fg::red
            << flairText;
    }

    std::cout
        << rang::fg::reset
        << rang::bg::reset
        << rang::style::reset
        << '\n'
        << std::endl;
}

void ConsoleApp::renderComment(const nlohmann::json& comment)
{

}

void ConsoleApp::printListing()
{
    if (!_listing || _currentPage.empty()) return;

    std::size_t idx = 0;
    for (const auto& item : _currentPage)
    {
        if (item.at("data").is_null())
        {
            continue;
        }

        const auto listkind = item.value("kind", "t3");
        if (listkind == "t3")
        {
            renderLink(item.at("data"), ++idx, false);
        }
        else
        {
            printError(fmt::format("unsupported list prefix '{}'", listkind));
        }
    }
}

void ConsoleApp::list(const std::string& cmdParams)
{
    static const std::vector<std::string> validTypes = { "new", "hot", "rising", "controversial", "top" };
    static const std::vector<std::string> validTValues = { "hour", "day", "week", "month", "year", "all" };

    Params listParams;
    SimpleArgs args{ cmdParams };
    std::string endpoint;

    if (const std::string subName = args.getNamedArgument("sub"); 
        !subName.empty())
    {
        if (boost::starts_with(subName, "/r/"))
        {
            endpoint = subName;
        }
        else
        {
            endpoint = fmt::format("/r/{}", subName);
        }
    }
    else if (_session->location() != "/")
    {
        endpoint = _session->location();
    }

    std::string listType;
    if (args.getPositionalCount() > 0)
    {
        auto temp = args.getPositional(0);

        if (std::find(std::begin(validTypes), 
            std::end(validTypes), temp) == std::end(validTypes))
        {
            ConsoleApp::printError(fmt::format("invalid list type '{}'", temp));
            ConsoleApp::printStatus(fmt::format("valid values: {}",
                boost::algorithm::join(validTypes,", ")));

            return;
        }

        endpoint.append(fmt::format("/{}", temp));
        listType.assign(temp);
    }
    else
    {
        const auto typeSetting = _settings.value("command.list.type", "hot");
        endpoint.append(fmt::format("/{}", typeSetting));
        listType.assign(typeSetting);
    }

    if (listType == "top" && args.hasArgument("t"))
    {
        const auto& tval = args.getNamedArgument("t");
        if (std::find(std::begin(validTValues), std::end(validTValues), tval)
                == std::end(validTValues))
        {
            ConsoleApp::printError(fmt::format("invalid t-value with 'top' param '{}'", tval));
            ConsoleApp::printStatus(fmt::format("valid values: {}",
                boost::algorithm::join(validTValues,", ")));

            return;
        }

        listParams.insert_or_assign("t", tval);
    }

    std::size_t limit = _settings.value("command.list.limit", 5u);
    if (args.hasArgument("limit"))
    {
        auto limitstr = args.getNamedArgument("limit");
        if (!utils::isNumeric(limitstr))
        {
            ConsoleApp::printError("parameter 'limit' has invalid value '" + limitstr + "'");
            return;
        }

        limit = static_cast<std::uint32_t>(std::stoul(limitstr));
    }

    auto listing = std::make_unique<Listing>(_session, endpoint, limit, listParams);
    if (auto page = listing->getFirstPage(); !page.empty())
    {
        ConsoleApp::printStatus(fmt::format("showing {} '{}' items from '{}'",
            limit, listType, endpoint ));

        std::cout << '\n';

        _listing = std::move(listing);
        _currentPage = std::move(page);

        printListing();
    }
}

void ConsoleApp::next(const std::string&)
{
    if (auto page = _listing->getNextPage(); !page.empty())
    {
        _currentPage = std::move(page);
        printListing();
    }
    else
    {
        ConsoleApp::printWarning("no more posts");
    }
}

void ConsoleApp::previous(const std::string&)
{
    if (auto page = _listing->getPreviousPage(); !page.empty())
    {
        _currentPage = std::move(page);
        printListing();
    }
    else
    {
        ConsoleApp::printWarning("no more previous posts");
    }
}

} // namespace arcc
