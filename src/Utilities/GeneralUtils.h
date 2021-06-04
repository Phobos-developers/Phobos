#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <cstring>
#include <iterator>
#include <vector>

#include "PhobosTemplate.h"

namespace GeneralUtils
{

	bool IsValidString(const char* str);
	
	const wchar_t* LoadStringOrDefault(char* key, const wchar_t* defaultValue);
	
	const wchar_t* LoadStringUnlessMissing(char* key, const wchar_t* defaultValue);
	
	std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
	
	const int GetRangedRandomOrSingleValue(const Point2D& range);
	
	template<typename T>
	void NumericValidCheck(T&& source, const char* pSection, const char* pTag, const T&& nDefaultValue,
		const T&& nMin = std::numeric_limits<T>::min(), const T&& nMax = std::numeric_limits<T>::max())
	{
		if (source < nMin || source>nMax)
		{
			Debug::Log("[Developer warning][%s]%s="string_formatter<T>
				" is invalid! Reset to "string_formatter<T>".\n",
				pSection, pTag, source, nDefaultValue);
			source = nDefaultValue;
		}
	}

	template<typename T>
	const double GetWarheadVersusArmor(WarheadTypeClass* pWH, T ArmorType)
	{
		return double(MapClass::GetTotalDamage(100, pWH, static_cast<int>(ArmorType), 0)) / 100.0;
	}
};