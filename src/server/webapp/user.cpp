#include "user.h"

#include "password.h"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <primitives/hash.h>

#include <Wt/WApplication.h>

#include <regex>
#include <unordered_set>

static const std::regex r_login("[a-z][a-z0-9_]+");

const std::unordered_set<String> reserved_words{
    "pub",
    "pvt",
    "com",
    "org",
    "loc",
    "enc",
};

bool is_reserved_user_name(const String &name)
{
    return
        //web_pages.find(name) != web_pages.end() ||
        reserved_words.find(name) != reserved_words.end()
        ;
}

std::tuple<bool, std::string> is_valid_login(const String &n)
{
    static const auto err = "Username should contain alphanumeric characters or undescore symbols "
        "starting with an alpha and 2-32 characters length";
    if (n.size() < 2 || n.size() > 32)
        return { false, err };
    if (!std::regex_match(n, r_login))
        return { false, err };
    return { true, "" };
}

std::tuple<bool, std::string> is_valid_email(const String &n)
{
    if (n.find('@') == n.npos)
        return { false, "Bad email" };
    if (std::count(n.begin(), n.end(), '@') != 1)
        return { false, "Bad email" };

    for (auto c : n)
    {
        if ((c > 0 && c < 127 && isalnum(c)) ||
            c == '.' ||
            c == '@' ||
            c == '_' ||
            c == '-'
            )
            continue;

        return { false, "Email may contain only [A-Za-z0-9@-_.]" };
    }
    return { true, "" };
}

std::tuple<Person, std::string /*cookie*/, std::string /*error*/> login(std::string name, std::string password, bool remember)
{
    boost::to_lower(name);

    auto &db = getAttendanceDatabase();

    Person p;
    p.login = name;
    if (!db.findPersonByLogin(p))
        return { p, "", "There is no such user with current name and password" };

    if (!check_password(password, p.password, p.salt))
        return { p, "", "Incorrect password" };

    auto cookie_expires = boost::posix_time::second_clock::local_time();
    int mult = 1;
    if (remember)
        mult = 30;
    cookie_expires += boost::posix_time::hours(mult * 24);

    while (1)
    {
        try
        {
            p.cookie = generate_random_alnum_sequence(USER_SESSION_COOKIE_LENGTH);
            p.cookie_expires = boost::posix_time::to_simple_string(cookie_expires);
            db.setCookie(p);
            break;
        }
        catch (const sqlpp::exception &)
        {
        }
    }

    auto seconds = 60 * 60 * 24;
    if (remember)
        seconds *= 30;
    Wt::WApplication::instance()->setCookie(COOKIE_USER_ID, std::to_string(p.id), seconds, "", "", false);
    Wt::WApplication::instance()->setCookie(COOKIE_USER_SESSION, p.cookie, seconds, "", "", false);

    db.loadPerson(p);

    return { p, p.cookie, "" };
}

Person loginByCookie(const std::string &id, const std::string &user_session)
{
    Person p;
    if (id.empty() || id.size() > 20 || user_session.empty())
        return p;

    try
    {
        p.id = std::stoll(id);
    }
    catch (...)
    {
        return p;
    }

    p.cookie = user_session;

    auto &db = getAttendanceDatabase();
    if (!db.findPersonByIdAndCookie(p))
    {
        p.id = 0;
        return p;
    }

    auto cookie_expires = boost::posix_time::time_from_string(p.cookie_expires);
    if (cookie_expires < boost::posix_time::second_clock::local_time())
    {
        db.dropCookie(p);
        // reset
        Wt::WApplication::instance()->removeCookie(COOKIE_USER_ID);
        Wt::WApplication::instance()->removeCookie(COOKIE_USER_SESSION);
    }

    db.loadPerson(p);

    return p;
}

void logout()
{
    Wt::WApplication::instance()->removeCookie(COOKIE_USER_ID);
    Wt::WApplication::instance()->removeCookie(COOKIE_USER_SESSION);
}

std::tuple<bool, std::string> register_user(Person &p)
{
    // prepare name
    boost::to_lower(p.login);
    if (auto t = is_valid_login(p.login); !std::get<0>(t))
        return { false, std::get<1>(t) };
    if (is_reserved_user_name(p.login))
        return { false, "User name is a reserved word" };

    // prepare email
    //boost::to_lower(email);
    //if (auto t = is_valid_email(email); !std::get<0>(t))
    //    return { false, std::get<1>(t) };

    if (auto t = is_valid_password(p.password); !std::get<0>(t))
        return { false, std::get<1>(t) };

    hash_password(p.password, p.salt);

    auto &db = getAttendanceDatabase();
    try
    {
        db.addPerson(p);
    }
    catch (...)
    {
        return { false, "User with such name already exists" };
    }

    return { true, "" };
}
