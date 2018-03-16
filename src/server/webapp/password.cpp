#include "password.h"

#include <primitives/hash.h>

#include <openssl/evp.h>

// 100 bytes will occupy 200 chars in db in hex format
#define USER_PASSWORD_SALT_LENGTH 100
#define USER_PASSWORD_LENGTH 100

String generate_salt(int len)
{
    auto salt = generate_strong_random_bytes(len);
    salt = bytes_to_string(salt);
    return salt;
}

String generate_password(const String &password, const String &salt)
{
    static const auto N = 16384;
    static const auto r = 8;
    static const auto p = 2;
    String key(USER_PASSWORD_LENGTH, 0);
    if (EVP_PBE_scrypt(password.c_str(), password.size(), (unsigned char *)salt.c_str(), salt.size(),
        N, r, p, 0 /* maxmem  = 32 MB by default */, (unsigned char *)&key[0], key.size()))
    {
        key = bytes_to_string(key);
        return key;
    }
    throw std::runtime_error("Error during password hash");
}

void hash_password(String &password, String &salt)
{
    salt = generate_salt(USER_PASSWORD_SALT_LENGTH);
    password = generate_password(password, salt);
}

bool check_password(const String &password_entered, const String &password_hash, const String &salt)
{
    return generate_password(password_entered, salt) == password_hash;
}

std::tuple<bool, std::string> is_valid_password(const String &p)
{
    if (p.size() < 6)
        return { false, "Minimum password length is 6 symbols" };
    int lower = 0;
    int upper = 0;
    int digit = 0;
    int special = 0;
    static const auto spaces_error = "Spaces are not allowed in password";
    for (auto &c : p)
    {
        if (c <= 0 || c >= 127)
            return { false, spaces_error };
        else if (islower(c))
            lower++;
        else if (isupper(c))
            upper++;
        else if (isdigit(c))
            digit++;
        else if (isspace(c) || iscntrl(c))
            return { false, spaces_error };
        else if (isprint(c))
            special++;
        else
            return { false, spaces_error };
    }
    bool ok = (lower || upper);// && digit;
    if (!ok)
        return { false, "Password must have at least one small and capital letter" };
    return { ok, "" };
}
