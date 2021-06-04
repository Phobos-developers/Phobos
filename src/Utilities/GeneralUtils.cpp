#include "GeneralUtils.h"
#include <Misc/Debug.h>
#include <ScenarioClass.h>

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

std::vector<CellStruct> GeneralUtils::AdjacentCellsInRange(unsigned int range)
{
	std::vector<CellStruct> result;

	for (CellSpreadEnumerator it(range); it; ++it)
		result.push_back(*it);

	return result;
}

const int GeneralUtils::GetRangedRandomOrSingleValue(const Point2D& range)
{
	return range.X >= range.Y ?
		range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
}

