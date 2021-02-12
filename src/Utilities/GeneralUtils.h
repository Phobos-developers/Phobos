#pragma once
#define NONE_STR "<none>"
#define NONE_STR2 "none"

bool IsValidString(const char* str);

const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue);