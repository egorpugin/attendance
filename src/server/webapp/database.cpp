/*
 * Copyright (C) 2016-2017, Egor Pugin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "database.h"

#include "exceptions.h"
#include "hash.h"
#include "http.h"
#include "server_options.h"

#include "schema.h"
#include <sqlpp11/custom_query.h>
#include <sqlpp11/sqlite3/sqlite3.h>
#include <sqlpp11/sqlpp11.h>
#include <sqlite3.h>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/nowide/fstream.hpp>

#include <shared_mutex>

#include <primitives/log.h>
//DECLARE_STATIC_LOGGER(logger, "db");

AttendanceDatabase &getAttendanceDatabase()
{
    static const auto path = getProgramOptions().getDbPath();
    thread_local
    AttendanceDatabase db(path);
    return db;
}

AttendanceDatabase::AttendanceDatabase(const path &name)
{
    fn = name;
    sql::connection_config config;
    config.path_to_database = normalize_path(fn);
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    //config.debug = true;

    db = std::make_unique<sql::connection>(config);
}

void AttendanceDatabase::addGroup(const String &s) const
{
    if (s.empty())
        return;
    const auto g = db::Groups{};
    (*db)(insert_into(g).set(
        g.groupName = s
    ));
}

Groups AttendanceDatabase::getGroups() const
{
    Groups m;
    const auto groups = db::Groups{};
    for (const auto& row : (*db)(select(all_of(groups)).from(groups).unconditionally().order_by(groups.groupName.asc())))
    {
        Group g;
        g.id = row.groupId;
        g.name = row.groupName;
        m.push_back(g);
    }
    return m;
}

void AttendanceDatabase::addSubject(const String &s) const
{
	if (s.empty())
		return;
	const auto g = db::Subject{};
	(*db)(insert_into(g).set(
		g.subjectName = s
	));
}

Subjects AttendanceDatabase::getSubjects() const
{
	Subjects m;
	const auto s = db::Subject{};
	for (const auto& row : (*db)(select(all_of(s)).from(s).unconditionally().order_by(s.subjectName.asc())))
	{
		Subject g;
		g.id = row.subjectId;
		g.name = row.subjectName;
		m.push_back(g);
	}
	return m;
}

void AttendanceDatabase::addCourse(db_id sid, db_id tid, const std::set<db_id> &gids, int sem) const
{
	for (auto &gid : gids)
	{
		const auto c = db::Course{};
		(*db)(insert_into(c).set(
			c.subjectId = sid,
			c.teacherId = tid,
			c.groupId = gid,
			c.semester = sem
		));
	}
}

db_id AttendanceDatabase::addPerson(const Person &p) const
{
    const auto person = db::Person{};
    return (*db)(insert_into(person).set(
        person.login = p.login,
        person.password = p.password,
        person.salt = p.salt,
        person.surname = p.surname,
        person.firstName = p.first_name,
        person.middleName = p.middle_name
    ));
}

bool AttendanceDatabase::findPersonByLogin(Person &p) const
{
    const auto person = db::Person{};
    auto obj = (*db)(select(all_of(person)).from(person).where(person.login == p.login));
    bool found = !obj.empty();
    for (const auto& row : obj)
    {
        p.id = row.personId;
        p.password = row.password;
        p.salt = row.salt;
        p.cookie = row.cookie;
        p.cookie_expires = row.cookieExpires;
    }
    return found;
}

bool AttendanceDatabase::findPersonByIdAndCookie(Person &p) const
{
    const auto person = db::Person{};
    auto obj = (*db)(select(all_of(person)).from(person).where(person.personId == p.id && person.cookie == p.cookie));
    bool found = !obj.empty();
    for (const auto& row : obj)
    {
        p.cookie_expires = row.cookieExpires;
    }
    return found;
}

Persons AttendanceDatabase::getTeachers() const
{
	Persons pps;
	const auto person = db::Person{};
	for (const auto& row : (*db)(select(all_of(person)).from(person).where(person.typeId > (int)PersonType::Normal)
		.order_by(person.surname.asc(), person.firstName.asc(), person.middleName.asc())))
	{
		Person p;
		p.id = row.personId;
		p.login = row.login;
		p.surname = row.surname;
		p.first_name = row.firstName;
		p.middle_name = row.middleName;
		p.type_id = (PersonType)row.typeId.value();
		pps.push_back(p);
	}
	return pps;
}

void AttendanceDatabase::setCookie(const Person &p) const
{
    const auto person = db::Person{};
    (*db)(update(person).set(
        person.cookie = p.cookie,
        person.cookieExpires = p.cookie_expires).where(person.personId == p.id));
}

void AttendanceDatabase::dropCookie(const Person &p) const
{
    const auto person = db::Person{};
    (*db)(update(person).set(
        person.cookie = "",
        person.cookieExpires = "").where(person.personId == p.id));
}

void AttendanceDatabase::loadPerson(Person &p) const
{
    const auto person = db::Person{};
    for (const auto& row : (*db)(select(all_of(person)).from(person).where(person.personId == p.id)))
    {
		p.id = row.personId;
        p.login = row.login;
        p.surname = row.surname;
        p.first_name = row.firstName;
        p.middle_name = row.middleName;
        p.type_id = (PersonType)row.typeId.value();
    }
}

CheckStatus AttendanceDatabase::checkIn(const CheckInInfo &i)
{
    const auto j = db::Journal{};

    struct class_time
    {
        boost::posix_time::time_duration begin;
        boost::posix_time::time_duration end;
    };

    static const std::vector<class_time> cts = []{
        static const std::pair<String, String> cts[] = {
            { "8:30", "10" },
            { "10:15", "11:45" },
            { "12:30", "14" },
            { "14:15", "15:45" },
            { "16", "17:30" },
            { "17:45", "19:15" },
            };
        std::vector<class_time> cts2;
        for (auto &ct : cts)
        {
            class_time c;
            c.begin = boost::posix_time::duration_from_string(ct.first);
            c.end = boost::posix_time::duration_from_string(ct.second);
            cts2.push_back(c);
        }
        return cts2;
    }();

    auto time = boost::posix_time::second_clock::local_time();
    auto t = time.time_of_day();
    auto d = time.date();
    for (auto &ct : cts)
    {
        if (t >= ct.begin && t <= ct.end)
        {
            int64_t ts = boost::posix_time::to_time_t(time);
            auto b = boost::posix_time::ptime(d) + ct.begin;
            auto e = boost::posix_time::ptime(d) + ct.end;

            auto c = (*db)(select(count(j.personId)).from(j).where(
                j.timestamp >= boost::posix_time::to_time_t(b) &&
                j.timestamp <= boost::posix_time::to_time_t(e) &&
                j.personId == i.person_id
                )).front().count;
            if (c > 0)
                return CheckStatus::AlreadyChecked;

            (*db)(insert_into(j).set(
                j.personId = i.person_id,
                j.timestamp = ts
            ));

            return CheckStatus::Ok;
        }
    }

    return CheckStatus::InappropriateTime;
}

#include <unicode/ustring.h>

String Person::getFio() const
{
    String s = surname + " ";
    /*auto fn = icu::UnicodeString::fromUTF8(first_name.c_str());
    fn.tempSubString(0, 1).toUTF8String(s);
    s += ".";
    auto mn = icu::UnicodeString::fromUTF8(middle_name.c_str());
    mn.tempSubString(0, 1).toUTF8String(s);
    s += ".";*/
    s += first_name + " ";
    s += middle_name + " ";
    return s;
}
