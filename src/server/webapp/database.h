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

#pragma once

#include "filesystem.h"

#include <sqlpp11/sqlite3/sqlite3.h>
#include <sqlpp11/sqlpp11.h>

#include <chrono>
#include <memory>
#include <vector>

namespace sql = sqlpp::sqlite3;

using db_id = int64_t;

struct Group
{
    db_id id;
    String name;
    int type_id;
};

using Groups = std::vector<Group>;
using GroupMap = std::unordered_map<int, Group>;

struct Subject
{
    db_id id;
    String name;
};

using Subjects = std::vector<Subject>;

enum class PersonType : int
{
    Normal          = 1,
    Teacher         = 10,
    Administrator   = 100,
    Root            = 1000,
};

struct Person
{
    db_id id = 0;

    String login;
    String password;
    String salt;
    String cookie;
    String cookie_expires;

    String surname;
    String first_name;
    String middle_name;

    Groups groups;

    PersonType type_id = PersonType::Normal;

    String getFio() const;
};

using Persons = std::vector<Person>;

struct CheckInInfo
{
    db_id person_id;
    int64_t timestamp;
};

enum class CheckStatus
{
    Ok,
    AlreadyChecked,
    InappropriateTime,
};

class AttendanceDatabase
{
public:
    AttendanceDatabase(const path &name);
    AttendanceDatabase(const AttendanceDatabase &) = delete;
    AttendanceDatabase &operator=(const AttendanceDatabase &) = delete;

    void addGroup(const String &s) const;
    Groups getGroups() const;

    void addSubject(const String &s) const;
    Subjects getSubjects() const;

    void addCourse(db_id sid, db_id tid, const std::set<db_id> &gids, int sem) const;

    db_id addPerson(const Person &p) const;
    bool findPersonByLogin(Person &p) const;
    bool findPersonByIdAndCookie(Person &p) const;
    void loadPerson(Person &p) const;
    Persons getTeachers() const;

    CheckStatus checkIn(const CheckInInfo &i);

    void setCookie(const Person &p) const;
    void dropCookie(const Person &p) const;

    std::unique_ptr<sql::connection> db;

protected:
    path fn;
};

AttendanceDatabase &getAttendanceDatabase();
