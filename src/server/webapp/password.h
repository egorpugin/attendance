#pragma once

#include <primitives/string.h>

void hash_password(String &password, String &salt);
bool check_password(const String &password_entered, const String &password_hash, const String &salt);

std::tuple<bool, std::string> is_valid_password(const String &p);

