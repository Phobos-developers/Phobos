#pragma once

constexpr auto NONE_STR = "<none>";
constexpr auto NONE_STR2 = "none";

class GeneralUtils
{
public:
    static bool IsValidString(const char* str);
    static const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue);
};