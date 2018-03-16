#pragma once

#include "database.h"

#include <string>

#define COOKIE_USER_ID "user_id"
#define COOKIE_USER_SESSION "user_session"
#define USER_SESSION_COOKIE_LENGTH 80

std::tuple<Person, std::string /*cookie*/, std::string /*error*/>
login(std::string name, std::string password, bool remember = false);

Person loginByCookie(const std::string &id, const std::string &cookie);

void logout();

std::tuple<bool /*ok*/, std::string /*error*/>
register_user(Person &p);
