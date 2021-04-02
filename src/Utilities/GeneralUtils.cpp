#include "GeneralUtils.h"

bool GeneralUtils::IsValidString(const char* str)
{
    return str != nullptr
        && strlen(str) != 0
        && !INIClass::IsBlank(str);
}

const wchar_t* GeneralUtils::LoadStringOrDefault(char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadString(key);
	else
		return defaultValue;
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(char* key, const wchar_t* defaultValue)
{
	return wcsstr(LoadStringOrDefault(key, defaultValue), L"MISSING:") ? defaultValue : LoadStringOrDefault(key, defaultValue);
}

std::vector<CellStruct> GeneralUtils::CellSpreadAffectedCells(const double spread)
{
	std::vector<CellStruct> result;
	auto const range = static_cast<size_t>(spread + 0.99);

	for (CellSpreadEnumerator it(range); it; ++it) {
		result.push_back(*it);
	}

	return result;
}